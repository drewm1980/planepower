#ifndef __WINCHCONTROL__
#define __WINCHCONTROL__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <stdlib.h>
#include <stdint.h>  /* int types with given size */

#include <ocl/OCL.hpp>

#include <fstream>

#include <epos2/Definitions.h>


/* EPOS modes of operation, firmware spec 14.1.59 (p.133, tbl. 72) */
#define E_HOMING      6 ///< EPOS operation mode: homing
#define E_POSITIONVELOCITY     3 ///< EPOS operation mode: profile velocity mode
#define E_POSITIONPROFILE     1 ///< EPOS operation mode: profile position mode
// the modes below should not be used by user, defined here only for
// completeness
#define E_POSITION     -1 ///< EPOS operation mode: position mode
#define E_VELOCITY     -2 ///< EPOS operation mode: velocity mode
#define E_CURRENT    -3 ///< EPOS operation mode: current mode
#define E_MASTERENCODER -5 ///< EPOS operation mode:internal
#define E_STEPDIRECTION -6 ///< EPOS operation mode:internal


typedef uint32_t DWORD; ///< \brief 32bit type for EPOS data exchange
typedef uint16_t WORD; ///< \brief 16bit type for EPOS data exchange
//! signed 8-bit integer
typedef int8_t INTEGER8;

//! signed 16-bit integer
typedef int16_t INTEGER16;

//! signed 32-bit integer
typedef int32_t INTEGER32;

//! unsigned 8-bit integer
typedef uint8_t UNSIGNED8;

//! unsigned 16-bit integer
typedef uint16_t UNSIGNED16;

//! unsigned 32-bit integer
typedef uint32_t UNSIGNED32;



using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

namespace OCL
{

	/// WinchControl class
	/**
	This class simulates the free motion of a ball attached to a pendulum.
	The pendulum motion is executed in the x=0 plane of the pendulum reference
	frame. The state of the ball in the pendulum plane is given by
	[theta,omega,alpha]. The position of the ball in the world frame is given by
	[x,y,z].
	The pendulum reference frame wrt to the world reference frame gives the pose
	of the pendulum motion plane wrt to the world.
	*/
	class WinchControl
		: public TaskContext
	{
	protected:
		/*********
		PROPERTIES
		*********/
		//! Example of a property
		Property<double>			_prop;

		/*********
		DATAPORTS
		*********/
		//! Input port
		InputPort<double>			_inPort;
		//! Output port
		OutputPort<double>			_outPort;

	private:
		/// Handle for port access
		void* keyHandle;
		int NodeId;
		// Error information about the executed function
		long unsigned int ErrorCode;
        	/** 
        	 * Opens the connection with the EPOS2 device
        	 */
		bool openDevice();
		unsigned int nodeId;
		bool move(int steps);
		bool vel(int vel);
		void setPositionProfile(int vel, int acc);
		void setVelocityProfile(int acc);
		void enableBrake();
		void disableBrake();
		int getPosition();
		int getVelocity();
		int getCurrent();

	public:
		WinchControl(std::string name);
		~WinchControl();
		bool		configureHook();
		bool		startHook();
		void		updateHook();
		void		stopHook();
		void		cleanUpHook();
		
	};
}
#endif // __WINCHCONTROL__
