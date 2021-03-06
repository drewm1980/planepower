// Description: Simulator for the indoors carousel
// Author:      Milan Vukov, milan.vukov@esat.kuleuven.be
// Date:        March 2014.

#ifndef INDOORS_CAROUSEL_SIMULATOR_HPP
#define INDOORS_CAROUSEL_SIMULATOR_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

/// Common header for the autogenerated code
#include "autogenerated/acado_common.h"
/// Include file with the simulator configuration
#include "autogenerated/sim_configuration.h"

//
// Define some common stuff
//
#define NX		ACADO_NX	// # differential variables
#define NXA		ACADO_NXA	// # algebraic variables
#define NU		ACADO_NU	// # control variables
#define NP		ACADO_NP	// # user parameters
#define NY		ACADO_NY	// # measurements, nodes 0.. N - 1
#define NYN		ACADO_NYN	// # measurements, last node
#define N 		ACADO_N		// # estimation intervals

typedef uint64_t TIME_TYPE;

// Here we import custom data type definitions
#include "mcuHandler/types/McuHandlerDataType.hpp"
#include "encoder/types/EncoderDataType.hpp"
#include "LEDTracker/types/LEDTrackerDataType.hpp"
#include "lineAngleSensor/types/LineAngleSensorDataType.hpp"
#include "winchControl/types/WinchControlDataType.hpp"

// Components' typekit
#include "types/IndoorsCarouselSimulatorDataType.hpp"

/// A buffer with functionality to delay samples
template<class T>
struct BufferWithDelay
{
	BufferWithDelay()
		: cnt_ts( 0 ), cnt_td( 0 ), ts( 0 ), td( 0 )
	{}

	void reset()
	{
		cnt_ts = 0;
		cnt_td = td;
	}

    bool update(T& _sample)
	{
		if (--cnt_ts <= 0)
		{
			cnt_ts = ts;

			samples.push_back( _sample );
		}

		if (--cnt_td <= 0)
		{
			cnt_td = ts;

			if (samples.size() > 0)
			{
				_sample = samples.front();
				samples.pop_front();
				return true;
			}
		}

		return false;
	}

	/// Counters, RW
	int cnt_ts, cnt_td;
	/// Sampling time in number of ticks of the simulator period, R
	int ts;
	/// Delay time in number of ticks of the simulator period, R
	int td;
	/// Samples
	std::deque< T > samples;
};

/// A class for simulation of the indoors carousel
class IndoorsCarouselSimulator
	: public RTT::TaskContext
{
public:
	/// Ctor
	IndoorsCarouselSimulator(std::string name);
	/// Dtor
	virtual ~IndoorsCarouselSimulator()
	{}
	
	/// Configuration hook.
	virtual bool configureHook( );
	/// Start hook.
	virtual bool startHook( );
	/// Update hook.
	virtual void updateHook( );
	/// Stop hook.
	virtual void stopHook( );

protected:
	//
	// Input ports
	//
	/// Port with controls
	RTT::InputPort< ControlSurfacesValues > portControls;
	/// control values holder
	ControlSurfacesValues controls;

	//
	// Output ports
	//

	/// Output trigger port, used to trigger the MHE
	RTT::OutputPort< TIME_TYPE > portTrigger;
	/// Trigger value
	TIME_TYPE trigger;
	/// MCU handler data outputs
	RTT::OutputPort< McuHandlerDataType > portMcuHandlerData;
	/// MCU handler data holder
	McuHandlerDataType mcuData;
	/// Encoder data port
	RTT::OutputPort< EncoderDataType > portEncoderData;
	/// Encoder data holder
	EncoderDataType encData;
	/// Camera data output
	RTT::OutputPort< LEDTrackerDataType > portLEDTrackerData;
	/// Camera data holder
	LEDTrackerDataType camData;
	/// LAS data output
	RTT::OutputPort< LineAngleSensorDataType > portLASData;
	/// LAS data holder
	LineAngleSensorDataType lasData;
	/// Winch data output
	RTT::OutputPort< WinchControlDataType > portWinchData;
	/// Winch data holder
	WinchControlDataType winchData;

	/// Debug port
	RTT::OutputPort< IndoorsCarouselSimulatorDataType > portDebugData;
	/// Debug data
	IndoorsCarouselSimulatorDataType debugData;

	//
	// Properties
	//
	
private:

	BufferWithDelay< McuHandlerDataType > mcu;
	BufferWithDelay< EncoderDataType > enc;
	BufferWithDelay< LineAngleSensorDataType > las;
	BufferWithDelay< LEDTrackerDataType > cam;
	BufferWithDelay< WinchControlDataType > winch;
	BufferWithDelay< uint64_t > mhe;

	void updateControls();
	void updateMcuData();
	void updateEncData();
	void updateLasData();
	void updateCamData();
	void updateWinchData();
	void updateMheTrigger();

	std::vector< double > integratorIO, outputs;

	bool firstRun;
	double samplingTime;
};

#endif // INDOORS_CAROUSEL_SIMULATOR_HPP
