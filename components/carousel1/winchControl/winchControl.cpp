#include "winchControl.hpp"

#include <rtt/Component.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <fstream>

using namespace std;
using namespace RTT;
using namespace RTT::os;

WinchControl::WinchControl(std::string name)
  : TaskContext(name, PreOperational),
    epos(RS232_DEV, RS232_BAUD, NODE_ID)
{
  addPort("data", portData)
    .doc("Data port");

  portData.setDataSample( data );
  portData.write( data );

  addOperation("changeTetherLength", &WinchControl::changeTetherLength, this, OwnThread)
    .doc("Change the tether length relatively, with input in meter. Positive means extending tether");

  addOperation("setTetherLength", &WinchControl::setTetherLength, this, OwnThread)
    .doc("Change the tether length absolute, with input in meter. This is an absolute value!");

  addOperation("getTetherLength", &WinchControl::getTetherLength, this, OwnThread)
    .doc("Get the current tether length in meter");

  addOperation("getPosition", &WinchControl::getPosition, this, OwnThread)
    .doc("Get the current position in ticks");

  addOperation("getVelocity", &WinchControl::getVelocity, this, OwnThread)
    .doc("Get the current velocity, in rpm?");
}


WinchControl::~WinchControl()
{}

bool WinchControl::configureHook()
{
    return true;
}

bool WinchControl::startHook()
{
  if(openDevice() == false)
    return false;
  
  double foo;
  if( loadTetherLength( foo ) )
  {
    log( Info ) << "My current tether length, based on previously stored tether length ticks, is " << REF_LENGTH << " m" << endlog();
  }
  else
    {
      log( Error ) << "Error in reading the tether length" << endlog();
      return false;
    }

  refTicks = foo;

    //  VCS_DefinePosition(keyHandle, NODE_ID, tether_length_ticks,&ErrorCode);

  disableBrake();
    
  epos.enableController();

  epos.enableMotor( epos.PROFILE_POSITION );

  setAbsolutePosition( refTicks );

  return true;
}

void  WinchControl::updateHook()
{
  uint64_t tickStart = TimeService::Instance()->getTicks();

  int32_t theta, omega, current;

  theta = epos.readPosition();
  omega = epos.readVelocity();
  current = epos.readCurrent();

  data.dbg_theta = theta;
  data.dbg_omega = omega;
  data.dbg_current = current;

  data.ts_trigger = tickStart;
  data.ts_elapsed = TimeService::Instance()->secondsSince( tickStart );

  portData.write( data );
}

void  WinchControl::stopHook()
{
  enableBrake();

  epos.close();
  epos.shutdown();
  epos.faultReset();

  // NOPE! We manually set the this value manually
  //  saveTetherLength((int) getPosition());
}

bool WinchControl::openDevice()
{
  // Initialize the serial connection
  epos.init();
  // Reset all faults, if they exist
  epos.faultReset();
  // TODO, for now we use the defaults that are in the drive
  // setPositionProfile(100, 3000);
  
  return true;
}

bool WinchControl::setRelativePosition(int steps)
{
    epos.setTargetProfilePosition( steps );
    epos.startProfilePosition(epos.RELATIVE, false, false);

    return true;
}

bool WinchControl::setAbsolutePosition(int steps)
{
    epos.setTargetProfilePosition( steps );
    epos.startProfilePosition(epos.ABSOLUTE, false, false);
    
    return true;
}

bool WinchControl::changeTetherLength(double delta)
{
#if WINCH_DEBUG != 666
  double length = getTetherLength() + delta;
  if (length < MIN_TETHER_LENGTH || length > MAX_TETHER_LENGTH)
    return false;
#endif // WINCH_DEBUG != 666
  
  return setRelativePosition( length2ticks( delta ) );
}

bool WinchControl::setTetherLength(double length)
{
#if WINCH_DEBUG != 666
  if (length < MIN_TETHER_LENGTH || length > MAX_TETHER_LENGTH)
    return false;
#endif // WINCH_DEBUG != 666

  int32_t ticks = length2ticks(length - REF_LENGTH);
  return setAbsolutePosition(ticks + refTicks);
}

double WinchControl::getTetherLength()
{
    return REF_LENGTH + ticks2length(getPosition() - refTicks);
}

void WinchControl::enableBrake()
{
  epos.setDigOut(BRAKE_DO, false, true, false);
}

void WinchControl::disableBrake()
{
  epos.setDigOut(BRAKE_DO, true, true, false);
}

int WinchControl::getPosition()
{
    return epos.readPosition();
}

int WinchControl::getVelocity()
{
    return epos.readVelocity();
}

double WinchControl::ticks2length(int ticks)
{
    return ((double) ticks)/((double) WINCH_SCALING);
}

int WinchControl::length2ticks(double length)
{
    double ticks = length*((double) WINCH_SCALING);
    return (int) ticks;
}

bool WinchControl::loadTetherLength(double& length)
{
    ifstream file( TETHER_LENGTH_FILENAME );
    string line;
    unsigned numRows = 1;
    unsigned numCols = 1;
    vector< vector< double > > data;

    if ( file.is_open() )
    {
        data.clear();

        while( getline(file, line) )
        {
            istringstream linestream( line );
            vector< double > linedata;
            double number;

            while( linestream >> number )
            {
                linedata.push_back( number );
            }

            if (linedata.size() != numCols && numCols > 0)
            {
                log( Error ) << "numCols is wrong: " << linedata.size() << " vs " << numCols << endl;

                file.close();

                return false;
            }

            data.push_back( linedata );
        }

        file.close();

        if (data.size() != numRows && numRows > 0)
        {
            log( Error ) << "numRows is wrong: " << data.size() << " vs " << numRows << endl;
            return false;
        }
        length = data[0][0];
    }
    else
    {
        log( Error ) << "Could not open file " << TETHER_LENGTH_FILENAME << endl;
        return false;
    }

    return true;
}


bool WinchControl::saveTetherLength(double length)
{
    ofstream file( TETHER_LENGTH_FILENAME );

    if ( file.is_open() )
    {
        file << length;
        file.close();
    }
    else
    {
        log( Error ) << "Could not open file " << TETHER_LENGTH_FILENAME << endl;
        return false;
    }

    return true;
}

//ORO_CREATE_COMPONENT( WinchControl )
ORO_LIST_COMPONENT_TYPE( WinchControl )
