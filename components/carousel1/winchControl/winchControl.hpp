#ifndef __WINCHCONTROL__
#define __WINCHCONTROL__

#include <rtt/TaskContext.hpp>
#include <rtt/Port.hpp>

#include "types/WinchControlDataType.hpp"

#include "Epos2.h"

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

#define PI 3.14159265359
#define DRUM_CIRCUMFERENCE PI*0.1
#define GEAR_RATIO 3.7
#define ENCODER_RESOLUTION 2000.0 // Number of encoder ticks per revolution

//#define WINCH_SCALING GEAR_RATIO*ENCODER_RESOLUTION/DRUM_CIRCUMFERENCE // The scaling to go from tether lengt in meter to encoder ticks
//#define WINCH_SCALING 2000.0*3.7/0.1/3.14159265359
#define WINCH_SCALING 50000.0/2.100 // The scaling to go from tether length in meter to encoder ticks (ticks/meter)
#define MIN_TETHER_LENGTH 0.1
#define MAX_TETHER_LENGTH 2.2

#define TETHER_LENGTH_FILENAME "tetherlengthticks.dat"

#define NODE_ID 1
#define RS232_DEV "/dev/ttyS3"
#define RS232_BAUD 115200
#define BRAKE_DO 4
#define REF_LENGTH 1.275

// Enable this for debugging purposes only
// and handle winch with care...
//#define WINCH_DEBUG 666

/// WinchControl class
class WinchControl
    : public RTT::TaskContext
{
public:
    WinchControl(std::string name);

    ~WinchControl();

    virtual bool configureHook();
    virtual bool startHook();
    virtual void updateHook();
    virtual void stopHook();

protected:

  RTT::OutputPort< WinchControlDataType > portData;
  WinchControlDataType data;

private:
    // Error information about the executed function
    bool openDevice();

    bool setRelativePosition(int steps);
    bool setAbsolutePosition(int steps);
  
    bool changeTetherLength(double delta);
    bool setTetherLength(double length);
    double getTetherLength();

    void enableBrake();
    void disableBrake();

    int getPosition();
    int getVelocity();

    int length2ticks(double length);
    double ticks2length(int ticks);

    bool loadTetherLength(double& length);
    bool saveTetherLength(double length);

  CEpos2 epos;

  // The encoder reading at REF_LENGTH
  int32_t refTicks;
};

#endif // __WINCHCONTROL__
