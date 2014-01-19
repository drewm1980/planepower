#ifndef __WINCHCONTROL__
#define __WINCHCONTROL__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>
#include <stdlib.h>
#include <stdint.h>  /* int types with given size */

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
  //    virtual void cleanupHook();

protected:

private:
    /// Handle for port access
    void* keyHandle;
    int NodeId;
    // Error information about the executed function
    long unsigned int ErrorCode;
    bool openDevice();
    unsigned int nodeId;
    bool setRelativePosition(int steps);
    bool setAbsolutePosition(int steps);
    bool setVelocity(int vel);
    bool changeTetherLength(double length);
    bool setTetherLength(double length);
    void setPositionProfile(int vel, int acc);
    void setVelocityProfile(int acc);
    void enableBrake();
    void disableBrake();
    int getPosition();
    int getVelocity();
    int getCurrent();
    bool loadTetherLength(double& length);
    bool saveTetherLength(double length);
    void setCurrentTetherLength(double length);
    double getTetherLength();
    int length2ticks(double length);
    double ticks2length(int ticks);
};

#endif // __WINCHCONTROL__
