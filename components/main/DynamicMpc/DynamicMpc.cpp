#include "DynamicMpc.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <algorithm>

using namespace std;
using namespace RTT;
using namespace RTT::os;

static bool isNaN(double* array, unsigned dim)
{
	for (unsigned i = 0; i < dim; ++i)
		if ((array[ i ] != array[ i ]) || (array[ i ] < -1e12) || (array[ i ] > 1e12))
			return true;

	return false;	
}

/// Check port connection
#define checkPortConnection( port ) \
	if (port.connected() == false) \
	{log( Error ) << "Port " << port.getName() << " is not connected." << endlog(); return false;}

#define DEBUG 1

/// For internal use
enum DynamicMpcErrorCodes
{
	ERR_OK = 0,
	ERR_NANS,
	ERR_QP_STATUS,
	ERR_FEEDBACK_SIZE,
	ERR_PREPARE_REF,
	ERR_DEADLINE,
	ERR_MHE_NOT_READY
};

DynamicMpc::DynamicMpc(std::string name)
	: TaskContext(name, PreOperational)
{
	//
	// Set the input ports
	//
	addEventPort("feedback", portFeedback)
		.doc("Feedback -- state estimate.");
	addPort("currentControls", portCurrentControls)
		.doc("Current controls -- from the MCU handler.");

	//
	// Set the output ports
	//
	addPort("controls", portControls)
		.doc("MPC controls");

	addPort("debugData", portDebugData)
		.doc("Debugging data");

	controls.reset();
	portControls.setDataSample( controls );
	portControls.write( controls );

	debugData.x_hat.resize(NX, 0.0);
	debugData.x.resize((N + 1) * NX, 0.0);
	debugData.u.resize(N * NU, 0.0);
	debugData.z.resize(N * NXA, 0.0);
	debugData.y.resize(N * NY, 0.0);
	debugData.yN.resize(NYN, 0.0);
//	debugData.S.resize(NY * NY, 0.0);
//	debugData.SN.resize(NYN * NYN, 0.0);

	portDebugData.setDataSample( debugData );
	portDebugData.write( debugData );
	
	//
	// Properties
	//
	refPrescaler = 1;
	addProperty("refPrescaler", refPrescaler)
		.doc("Reference sampling time in ticks of the NMPC sapling time");

	//
	// Operations
	//
	addOperation("refStart", &DynamicMpc::refStart, this, ClientThread)
		.doc("Start reference generator");
	addOperation("refStop", &DynamicMpc::refStop, this, ClientThread)
		.doc("Stop reference generator");
	
	// Set reference "generator" to false
	refActive = false;
	refCnt = refPrescalerCnt = 0;
}

bool DynamicMpc::configureHook()
{
	checkPortConnection( portFeedback );
	
	return true;
}

bool DynamicMpc::startHook()
{
	// Clean the ACADO solver structures
	memset(&acadoWorkspace, 0, sizeof( acadoWorkspace ));
	memset(&acadoVariables, 0, sizeof( acadoVariables ));

	// NOTE: This guy cleans everything
	initializeSolver();

	// Configure weights, from the header file
	prepareWeights();

	// Reset the controls that are going to be sent to outside
	controls.reset();

	runCnt = 0;

	refActive = false;
	refCnt = refPrescalerCnt = 0;

	return true;
}

void DynamicMpc::updateHook()
{
	uint64_t trigger, fdbStart, prepStart;

	debugData.ts_entry = TimeService::Instance()->getTicks();

	// Clear error codes
	errorCode = ERR_OK;
	mpcStatus = 0;

	portFeedback.read( feedback );
	trigger = feedback.ts_trigger;

	// This is a hack, more/less. Here is assumed that first MPC.NX components
	// of the state estimate are the same. In other words, MHE state estimate 
	// can be longer, but MPC with take first MPC.NX components.
	if (feedback.x_hat.size() < NX)
	{
		errorCode = ERR_FEEDBACK_SIZE;
		goto DynamicMpcUpdateHookExit;
	}
	for (unsigned i = 0; i < NX; acadoVariables.x0[ i ] = feedback.x_hat[ i ], i++);

	if (feedback.ready == false)
	{
		// Abort if MHE is not ready!
		errorCode = ERR_MHE_NOT_READY;
		goto DynamicMpcUpdateHookExit;
	}

#if 0
	// Here we can override the control surface angles with what is actually applied
	// to the plane
	portCurrentControls.read( currentControls );
	acadoVariables.x0[ idx_aileron ] = currentControls.ctrl.ua1;
	acadoVariables.x0[ idx_elevator ] = currentControls.ctrl.ue;
#endif


	if (prepareReference() == false)
	{
		errorCode = ERR_PREPARE_REF;
		goto DynamicMpcUpdateHookExit;
	}

	if (runCnt == 0)
	{
		prepareInitialGuess();
	}
	else
	{
		fdbStart = TimeService::Instance()->getTicks();
		mpcStatus = feedbackStep();
		debugData.exec_fdb = TimeService::Instance()->secondsSince( fdbStart );

		if (isNaN(acadoVariables.x, NX * (N + 1)) == true or
			isNaN(acadoVariables.u, NU * N) == true)
		{
			mpcStatus = -100;
			errorCode = ERR_NANS;
		}
		
		if (mpcStatus == 0 or mpcStatus == QPOASES_RET_MAX_NWSR)
		{
#if 1
			// Derivative mode
			controls.d_ua1 = controls.d_ua2 = acadoVariables.u[ idx_daileron ];
			controls.d_ue  = acadoVariables.u[ idx_delevator ];

			controls.der_ctrl = true;
#else
			// Mode where we send the reference which should appear after one sampling period
			controls.ua1 = controls.ua2 = acadoVariables.x[NX + idx_aileron];
			controls.ue  = acadoVariables.x[NX + idx_elevator];

			controls.d_ua1 = controls.d_ua2 = 0.0;
			controls.d_ue  = 0.0;

			controls.der_ctrl = false;
#endif
		}
		else
		{
			controls.reset();
		}

		portControls.write( controls );

		debugData.ready = mpcStatus ? false : true;
		debugData.kkt_value = getKKT();
		debugData.obj_value = getObjective();
		debugData.n_asc     = getNWSR();
		debugData.solver_status = mpcStatus;
	}

	//
	// Copy all debug data
	//
	prepareDebugData();

	//
	// Prepare for the next iteration; run the RTI preparation step
	//

	prepStart = TimeService::Instance()->getTicks();
	if ( runCnt )
	{
		shiftStates(2, 0, 0);
		shiftControls( 0 );
	}
	preparationStep();
	debugData.exec_prep = TimeService::Instance()->secondsSince( prepStart );

DynamicMpcUpdateHookExit:

	debugData.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portDebugData.write( debugData );

	if (mpcStatus and mpcStatus != QPOASES_RET_MAX_NWSR and errorCode == ERR_OK)
		errorCode = ERR_QP_STATUS;
	if (errorCode != ERR_OK)
		stop();

	if (runCnt <= (5 * N))
		++runCnt;

	double elapsed = TimeService::Instance()->secondsSince( trigger );
	if (runCnt > 5 and elapsed > mpc_sampling_time)
	{
		errorCode = ERR_DEADLINE;
		stop();
	}
}

void DynamicMpc::stopHook( )
{
	Logger::In in( getName() );

	controls.reset();
	portControls.write( controls );

	switch( errorCode )
	{
	case ERR_OK:
		log( Info ) << "Successfully stopped." << endlog();
		break;
	case ERR_NANS:
		log( Error ) << "Runtime counter " << runCnt
					 << ": NANs detected in the controller structures." << endlog();
		break;
	case ERR_QP_STATUS:
		log( Error ) << "Runtime counter " << runCnt
					 << ": solver returned error code " << mpcStatus << "." << endlog();
		break;
	case ERR_FEEDBACK_SIZE:
		log( Error ) << "Runtime counter " << runCnt
					 << "Feedback size is wrong." << endlog();
		break;
	case ERR_PREPARE_REF:
		log( Error ) << "Runtime counter " << runCnt
					 << "Preparation of reference failed." << endlog();
		break;
	case ERR_DEADLINE:
		log( Error ) << "Runtime counter " << runCnt
					 << "Deadline is missed." << endlog();
		break;
	case ERR_MHE_NOT_READY:
		log( Error ) << "Runtime counter " << runCnt
					 << "Estimator is not ready." << endlog();
		break;

	default:
		log( Error ) << "Runtime counter " << runCnt
					 << ": unknown error." << endlog();
	}
}

void DynamicMpc::cleanupHook( )
{}

void DynamicMpc::errorHook( )
{}

bool DynamicMpc::prepareInitialGuess( void )
{
	//
	// Initialize differential variables
	// NOTE Must be called AFTER first feedback arrives!
	//

	for (unsigned blk = 0; blk < N + 1; ++blk)
		for (unsigned el = 0; el < NX; ++el)
			acadoVariables.x[blk * NX + el] = ss_x[ el ];

	// Initialize cos_delta and sin_delta from feedback
	double angle = atan2(feedback.x_hat[idx_sin_delta], feedback.x_hat[idx_cos_delta]);
	for (unsigned blk = 0; blk < N + 1; ++blk)
	{
		acadoVariables.x[blk * NX + idx_cos_delta] = cos( angle );
		acadoVariables.x[blk * NX + idx_sin_delta] = sin( angle );
		angle += ss_x[ idx_ddelta ] * mpc_sampling_time;
	}
	
	//
	// Initialize algebraic variables
	//
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NXA; ++el)
			acadoVariables.z[blk * NXA + el] = ss_z[ el ];
	
	//
	// Initialize control variables
	//
	for (unsigned blk = 0; blk < N; ++blk)
		for (unsigned el = 0; el < NU; ++el)
			acadoVariables.u[blk * NU + el] = ss_u[ el ];

	return true;
}

bool DynamicMpc::prepareWeights( void )
{
	for (unsigned el = 0; el < NY; ++el)
		acadoVariables.W[el * NY + el] = mpc_weights[ el ];
	for (unsigned el = 0; el < NYN; ++el)
		acadoVariables.WN[el * NYN + el] = mpc_wn_multiplier * mpc_weights[ el ];
	
	return true;
}

void DynamicMpc::refStart( void )
{
	refCnt = refPrescalerCnt = 0;
	refActive = true;
}

void DynamicMpc::refStop( void )
{
	refCnt = refPrescalerCnt = 0;
	refActive = false;
}

bool DynamicMpc::prepareReference( void )
{
	if (refActive == false)
	{
		// Set precomputed steady state as a reference!!!
		for (unsigned blk = 0; blk < N; ++blk)
			for (unsigned el = 0; el < NX; ++el)
				acadoVariables.y[blk * NY + el] = ss_x[ el ];

		// Set cos_delta and sin_delta from feedback
		double angle = atan2(feedback.x_hat[idx_sin_delta], feedback.x_hat[idx_cos_delta]);
		for (unsigned blk = 0; blk < N; ++blk)
		{
			acadoVariables.y[blk * NY + idx_cos_delta] = cos( angle );
			acadoVariables.y[blk * NY + idx_sin_delta] = sin( angle );
//			angle += feedback.x_hat[ idx_ddelta ] * mpc_sampling_time;
			angle += ss_x[ idx_ddelta ] * mpc_sampling_time;
		}

		// Here I kinda assume that those are zero...
		for (unsigned blk = 0; blk < N; ++blk)
			for (unsigned el = 0; el < NU; ++el)
				acadoVariables.y[blk * NY + NX + el] = ss_u[ el ];
	
		for (unsigned el = 0; el < NX; ++el)
			acadoVariables.yN[ el ] = ss_x[ el ];
		acadoVariables.yN[ idx_cos_delta ] = cos( angle );
		acadoVariables.yN[ idx_sin_delta ] = sin( angle );
	}
	else
	{
		if (++refPrescalerCnt >= refPrescaler)
		{
			refPrescalerCnt = 0;
			refCnt = (refCnt + 1) % REF_NUM_POINTS;
		}
		
		// Shift the reference
		for (unsigned blk = 0; blk < N - 1; ++blk)
			for (unsigned el = 0; el < NY; ++el)
				acadoVariables.y[blk * NY + el] = acadoVariables.y[(blk + 1) * NY + el];

		for (unsigned el = 0; el < NYN; ++el)
			acadoVariables.y[(N - 1) * NY + el] = acadoVariables.yN[ el ];

		// Add the new reference point at the end of the horizon
		for (unsigned el = 0; el < NX; ++el)
			acadoVariables.yN[ el ] = references[ refCnt ].x[ el ];

		
		for (unsigned el = 0; el < NU; ++el)
			acadoVariables.y[(N - 1) * NY + NX + el] = references[ refCnt ].u[ el ];

		// Set cos_delta and sin_delta from feedback
		double angle = atan2(feedback.x_hat[idx_sin_delta], feedback.x_hat[idx_cos_delta]);
		for (unsigned blk = 0; blk < N; ++blk)
		{
			acadoVariables.y[blk * NY + idx_cos_delta] = cos( angle );
			acadoVariables.y[blk * NY + idx_sin_delta] = sin( angle );
//			angle += feedback.x_hat[ idx_ddelta ] * mpc_sampling_time;
			angle += ss_x[ idx_ddelta ] * mpc_sampling_time;
		}

		acadoVariables.yN[ idx_cos_delta ] = cos( angle );
		acadoVariables.yN[ idx_sin_delta ] = sin( angle );
	}

	return true;
}

bool DynamicMpc::prepareDebugData( void )
{
	debugData.x_hat.assign(acadoVariables.x0, acadoVariables.x0 + NX);
	
	debugData.x.assign(acadoVariables.x, acadoVariables.x + (N + 1) * NX);
	debugData.u.assign(acadoVariables.u, acadoVariables.u + N * NU);
	debugData.z.assign(acadoVariables.z, acadoVariables.z + N * NXA);
	debugData.y.assign(acadoVariables.y, acadoVariables.y + N * NY);
	debugData.yN.assign(acadoVariables.yN, acadoVariables.yN + NYN);
//	debugData.S.assign(acadoVariables.W, acadoVariables.W + NY * NY);
//	debugData.SN.assign(acadoVariables.WN, acadoVariables.WN + NYN * NYN);

	debugData.ts_trigger = feedback.ts_trigger;

	return true;
}

ORO_LIST_COMPONENT_TYPE( DynamicMpc )
//ORO_CREATE_COMPONENT( DynamicMpc )
