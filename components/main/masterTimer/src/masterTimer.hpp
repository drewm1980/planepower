#ifndef __MASTERTIMER__
#define __MASTERTIMER__

#include <fstream>
#include <stdint.h>

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <ocl/OCL.hpp>

using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

//typedef RTT::os::TimeService::ticks TIME_TYPE;
typedef uint64_t TIME_TYPE;

namespace OCL
{
    /**
	 * See manifest.xml for description
   */
    class MasterTimer
        : public TaskContext
    {
    protected:
#define CLOCK_COUNT 2
		union{
			double target_hz[CLOCK_COUNT];
			struct {
				double _imu_target_hz;
				double _camera_target_hz;
			};
		};
		int dividers[CLOCK_COUNT];

        OutputPort<TIME_TYPE> _imuClock;
        OutputPort<TIME_TYPE> _cameraClock;
        OutputPort<TIME_TYPE> _masterClock;
	OutputPort<TIME_TYPE> *portPointers[CLOCK_COUNT];
	OutputPort<int>	_imuCameraRatio;

	InputPort<double> _deltaIn;
	OutputPort<double> _deltaOut;
	double delta;

    private:
		uint64_t base_clock_index;

    public:
        MasterTimer(std::string name);
        ~MasterTimer();
        bool        configureHook();
        bool        startHook();
        void        updateHook();
        void        stopHook();
        void        cleanUpHook();
        
    };
}
#endif // __MASTERTIMER__
