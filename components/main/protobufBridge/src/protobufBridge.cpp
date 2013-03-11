#include "protobufBridge.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ProtobufBridge)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
  ProtobufBridge::ProtobufBridge(std::string name)
    : TaskContext(name)
  {
    ports()->addEventPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
								  ",dx,dy,dz"
								  ",e11,e12,e13,e21,e22,e23,e31,e32,e33"
								  ",w1,w2,w3"
								  ",delta,ddelta,ur,up"); // We ONLY use the first 18 states from this;
    // mismatch in u has no effect on us.
                                                                                         
    ports()->addPort("portMHEFullStateVector", portMheFullStateVector).doc( "MHE: all states over the horizon." );
    ports()->addPort("portMHEFullControlVector", portMheFullControlVector).doc( "MHE: all controls over the horizon." );
                
    ports()->addPort("portMPCFullStateVector", portMpcFullStateVector).doc( "MPC: all states over the horizon." );
    ports()->addPort("portMPCFullControlVector", portMpcFullControlVector).doc( "MPC: all controls over the horizon." );
    
    ports()->addPort("portMeasurementsPast", portMeasurementsPast).doc( "MHE measurements on 1. N nodes." );
    ports()->addPort("portMeasurementsCurrent", portMeasurementsCurrent).doc( "MHE measurements on N + 1st node." );
  }

  ProtobufBridge::~ProtobufBridge()
  {
  }

  bool  ProtobufBridge::configureHook()
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    X.resize(NSTATES,0.0);
                
    mheFullStateVector.resize(   (NHORIZON + 1) * NSTATES,   0.0);
    mheFullControlVector.resize(  NHORIZON      * NCONTROLS, 0.0);
                
    mpcFullStateVector.resize(   (NHORIZON + 1) * NSTATES,   0.0);
    mpcFullControlVector.resize(  NHORIZON      * NCONTROLS, 0.0);
    
    measurementsPast.resize(	NHORIZON * NY, 0.0);
    measurementsCurrent.resize(	NYN, 0.0);

    mmh.clear_mhehorizon();
    mmh.clear_mpchorizon();
    mmh.clear_measurementshorizon();
    for (int k=0; k<NHORIZON+1; k++){
        mmh.add_mhehorizon();
        mmh.add_mpchorizon();
        mmh.add_measurementshorizon();
    }

    context = new zmq::context_t(1);

    return true;
  }

  bool  ProtobufBridge::startHook()
  {
    socket = new zmq::socket_t(*context,ZMQ_PUB);
    socket->bind("tcp://*:5563");

    return true;
  }

  void  ProtobufBridge::updateHook()
  {
    _stateInputPort.read( X );
                
    portMheFullStateVector.read( mheFullStateVector );
    portMheFullControlVector.read( mheFullControlVector );
                
    portMpcFullStateVector.read( mpcFullStateVector );
    portMpcFullControlVector.read( mpcFullControlVector );
    
    portMeasurementsPast.read( measurementsPast );
    portMeasurementsCurrent.read( measurementsCurrent );

    // set some constants
    mmh.set_carouselarmheight(2.0);
    mmh.set_visspan(0.96);
    mmh.set_rarm(1.085);
    mmh.set_zt(-0.05);

    MheMpc::DaePlus *daeplus;

    // write the "current state" field
    MheMpc::Dae *dae = mmh.mutable_currentstate();
    DiffStateVec * x = (DiffStateVec*) &(mpcFullStateVector[0]);
    ControlVec   * u = (ControlVec*)   &(mpcFullControlVector[0]);
    toDae(dae, x, u);

    // set mhe horizon
    for (int k=0; k<NHORIZON+1; k++){
        x = (DiffStateVec*) &(mheFullStateVector[k*NSTATES]);
        u = (ControlVec*)   &(mheFullControlVector[k*NCONTROLS]);
        double transparency = 0.2;
        if (k==NHORIZON) transparency = 1.0;
        daeplus = mmh.mutable_mhehorizon(k);
        dae = daeplus->mutable_dae();
        if (k<NHORIZON)
            toDae(dae, x, u);
        else
            toDae(dae, x, NULL);
        daeplus->set_kitetransparency(transparency);
        daeplus->set_linetransparency(transparency);
    }

    // set mpc horizon
    for (int k=0; k<NHORIZON+1; k++){
        x = (DiffStateVec*) &(mpcFullStateVector[k*NSTATES]);
        u = (ControlVec*)   &(mpcFullControlVector[k*NCONTROLS]);
        double transparency = 0.2;
        if (k==0) transparency = 1.0;
        daeplus = mmh.mutable_mpchorizon(k);
        dae = daeplus->mutable_dae();
        if (k<NHORIZON)
            toDae(dae, x, u);
        else
            toDae(dae, x, NULL);
        daeplus->set_kitetransparency(transparency);
        daeplus->set_linetransparency(transparency);
    }

    // set measurement horizon
    for (int k=0; k<NHORIZON; k++){
        fromMeasurementsXVec((mmh.mutable_measurementshorizon(k))->mutable_measurementsx(),
                             (MeasurementsXVec*)&(measurementsPast[k*NY]));
        fromMeasurementsUVec((mmh.mutable_measurementshorizon(k))->mutable_measurementsu(),
                             (MeasurementsUVec*)&(measurementsPast[k*NY+NYN]));
    }
    fromMeasurementsXVec((mmh.mutable_measurementshorizon(NHORIZON))->mutable_measurementsx(),
                         (MeasurementsXVec*)&(measurementsCurrent[0]));

    // set current measurement
    fromMeasurementsXVec(mmh.mutable_measurementscurrentx(),
                         (MeasurementsXVec*)&(measurementsCurrent[0]));
    fromMeasurementsXVec(mmh.mutable_measurementslastlatest()->mutable_measurementsx(),
                         (MeasurementsXVec*)&(measurementsPast[NY*(NHORIZON-1)]));
    fromMeasurementsUVec(mmh.mutable_measurementslastlatest()->mutable_measurementsu(),
                         (MeasurementsUVec*)&(measurementsPast[NY*(NHORIZON-1)+NYN]));


    if (!mmh.SerializeToString(&X_serialized)) {
      cerr << "Failed to serialize mmh." << endl;
      return;
    }
    s_sendmore(*socket, "mhe-mpc-horizons");
    s_send(*socket, X_serialized);
  }

  void  ProtobufBridge::stopHook()
  {
    delete socket;
  }

  void  ProtobufBridge::cleanUpHook()
  {
    delete context;
    google::protobuf::ShutdownProtobufLibrary(); // optional
  }

  void ProtobufBridge::toDae(MheMpc::Dae * dae, const DiffStateVec * x, const ControlVec * u){
      fromDiffStateVec(dae->mutable_diffstates(), x);
      if (u != NULL)
          fromControlVec(dae->mutable_controls(), u);
  }

  void ProtobufBridge::fromDiffStateVec(MheMpc::DifferentialStates *proto, const DiffStateVec *data)
  {
    proto->set_x( data->x );
    proto->set_y( data->y );
    proto->set_z( data->z );
    proto->set_dx( data->dx );
    proto->set_dy( data->dy );
    proto->set_dz( data->dz );
    proto->set_e11( data->e11 );
    proto->set_e12( data->e12 );
    proto->set_e13( data->e13 );
    proto->set_e21( data->e21 );
    proto->set_e22( data->e22 );
    proto->set_e23( data->e23 );
    proto->set_e31( data->e31 );
    proto->set_e32( data->e32 );
    proto->set_e33( data->e33 );
    proto->set_wx( data->wx );
    proto->set_wy( data->wy );
    proto->set_wz( data->wz );
    proto->set_delta( data->delta );
    proto->set_ddelta( data->ddelta );
    proto->set_ur( data->ur );
    proto->set_up( data->up );
  }
  
  void ProtobufBridge::fromAlgVarVec(MheMpc::AlgebraicVars *proto, const AlgVarVec *data)
  {
  }
  
  void ProtobufBridge::fromControlVec(MheMpc::Controls *proto, const ControlVec *data)
  {
    proto->set_dddelta( data->dddelta );
    proto->set_dur( data->dur );
    proto->set_dup( data->dup );
  }
  
  void ProtobufBridge::fromParamVec(MheMpc::Parameters *proto, const ParamVec *data)
  {
  }

  void ProtobufBridge::fromMeasurementsXVec(MheMpc::MeasurementsX *proto, const MeasurementsXVec *data){
    proto->set_uvc1m1_0(data->uvC1M1_0);
    proto->set_uvc1m1_1(data->uvC1M1_1);
    proto->set_uvc1m2_0(data->uvC1M2_0);
    proto->set_uvc1m2_1(data->uvC1M2_1);
    proto->set_uvc1m3_0(data->uvC1M3_0);
    proto->set_uvc1m3_1(data->uvC1M3_1);
    proto->set_uvc2m1_0(data->uvC2M1_0);
    proto->set_uvc2m1_1(data->uvC2M1_1);
    proto->set_uvc2m2_0(data->uvC2M2_0);
    proto->set_uvc2m2_1(data->uvC2M2_1);
    proto->set_uvc2m3_0(data->uvC2M3_0);
    proto->set_uvc2m3_1(data->uvC2M3_1);
    proto->set_wimu_0  (data->wIMU_0  );
    proto->set_wimu_1  (data->wIMU_1  );
    proto->set_wimu_2  (data->wIMU_2  );
    proto->set_aimu_0  (data->aIMU_0  );
    proto->set_aimu_1  (data->aIMU_1  );
    proto->set_aimu_2  (data->aIMU_2  );
    proto->set_delta   (data->delta   );
    proto->set_ur      (data->ur      );
    proto->set_up      (data->up      );
  }

  void ProtobufBridge::fromMeasurementsUVec(MheMpc::MeasurementsU *proto, const MeasurementsUVec *data){
    proto->set_dddelta(data->dddelta);
    proto->set_dur    (data->dur    );
    proto->set_dup    (data->dup    );
  };

}//namespace

