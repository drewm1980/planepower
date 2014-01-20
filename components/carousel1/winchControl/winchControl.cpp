#include "winchControl.hpp"
#include <fstream>

using namespace std;
using namespace RTT;

WinchControl::WinchControl(std::string name)
    : TaskContext(name)
{
    addOperation("changeTetherLength", &WinchControl::changeTetherLength, this)
    .doc("Change the tether length relatively, with input in meter. Positive means extending tether");

    addOperation("setTetherLength", &WinchControl::setTetherLength, this)
    .doc("Change the tether length absolute, with input in meter. This is an absolute value!");

    addOperation("setPositionProfile", &WinchControl::setPositionProfile, this)
    .doc("Set the position profile, first max velocity in rpm???, second max acceleration in rpm^2??");

    addOperation("setCurrentTetherLength", &WinchControl::setCurrentTetherLength,this)
    .doc("Specify the current tether length. This means changing the current tetherlength in software, but not in hardware.");

    addOperation("setVelocityProfile", &WinchControl::setVelocityProfile,this)
    .doc("Set the velocity profile, max acceleration in rpm^2??");

    addOperation("getPosition", &WinchControl::getPosition, this)
    .doc("Get the current position in ticks");

    addOperation("getTetherLength", &WinchControl::getTetherLength, this)
    .doc("Get the current tether length in meter");

    addOperation("getVelocity", &WinchControl::getVelocity, this)
    .doc("Get the current velocity, in rpm?");

    addOperation("getCurrent", &WinchControl::getCurrent,this)
    .doc("Get the current current");

    addOperation("enableBrake", &WinchControl::enableBrake,this)
    .doc("Enable the brake");

    addOperation("disableBrake", &WinchControl::disableBrake,this)
    .doc("Disable the brake");

    NodeId = 0;
}


WinchControl::~WinchControl()
{}

bool  WinchControl::configureHook()
{
    return true;
}

bool  WinchControl::startHook()
{
    if(openDevice() == false)
        return false;
    
    double tether_length_ticks;
    if( loadTetherLength( tether_length_ticks ) )
    {
      log( Info ) << "My current tether length, based on previously stored tether length ticks, is " << ticks2length(tether_length_ticks) << " m" << endlog();
    }
    //  VCS_DefinePosition(keyHandle, NodeId, tether_length_ticks,&ErrorCode);
    return true;
}

void  WinchControl::updateHook()
{}

void  WinchControl::stopHook()
{
    int isInFault;
    //  VCS_GetFaultState(keyHandle, NodeId, &IsInFault, &ErrorCode);
    if(isInFault)
    {
        //VCS_ClearFault(keyHandle, NodeId, &ErrorCode);
    }
    enableBrake();
    saveTetherLength((int) getPosition());
    //  VCS_SetDisableState(keyHandle, NodeId, &ErrorCode);
    //VCS_CloseDevice(keyHandle, &ErrorCode);
}


bool WinchControl::setRelativePosition(int steps)
{
    if(ticks2length(steps+getPosition()) < MIN_TETHER_LENGTH || ticks2length(steps+getPosition()) > MAX_TETHER_LENGTH)
    {
        cout << "Your move is invalid. Tether length should be between " << MIN_TETHER_LENGTH << " and " << MAX_TETHER_LENGTH << " m" << endl;
        return false;
    }
    //  if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONPROFILE,&ErrorCode) ){
    //return false;
    //}
    //if( !VCS_MoveToPosition(keyHandle, NodeId, steps, 0, 1, &ErrorCode ) ){
    //  return false;
    //}
    return true;
}

bool WinchControl::setAbsolutePosition(int steps)
{
    if(ticks2length(steps) < MIN_TETHER_LENGTH || ticks2length(steps) > MAX_TETHER_LENGTH)
    {
        cout << "Your move is invalid. Tether length should be between " << MIN_TETHER_LENGTH << " and " << MAX_TETHER_LENGTH << " m" << endl;
        return false;
    }
    // if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONPROFILE,&ErrorCode) )
    // {
    //     return false;
    // }
    // if( !VCS_MoveToPosition(keyHandle, NodeId, steps, 1, 1, &ErrorCode ) )
    // {
    //     return false;
    // }
    return true;
}

bool WinchControl::setVelocity(int vel)
{
    // if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONVELOCITY,&ErrorCode) )
    // {
    //     return false;
    // }
    // if( !VCS_MoveWithVelocity(keyHandle, NodeId, vel, &ErrorCode ) )
    // {
    //     return false;
    // }
    return true;
}

bool WinchControl::changeTetherLength(double length)
{
    double ticks = length2ticks(length);
    return setRelativePosition((int) ticks);
}

bool WinchControl::setTetherLength(double length)
{
    double ticks = length2ticks(length);
    return setAbsolutePosition((int) ticks);
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

void WinchControl::setPositionProfile(int vel, int acc)
{
  //    VCS_SetPositionProfile(keyHandle, NodeId, vel, acc, acc, &ErrorCode);
}

void WinchControl::setVelocityProfile(int acc)
{
  //    VCS_SetVelocityProfile(keyHandle, NodeId, acc, acc, &ErrorCode);
}

void WinchControl::setCurrentTetherLength(double length)
{
    int tether_length_ticks = length2ticks(length);
    //    VCS_DefinePosition(keyHandle, NodeId, tether_length_ticks,&ErrorCode);
}

void WinchControl::enableBrake()
{
    uint16_t DigitalOutputNb = 4;
    uint16_t Configuration = 14;
    bool State = false;
    bool Mask = true;
    bool Polarity = 0;
    //    VCS_DigitalOutputConfiguration(keyHandle, NodeId, DigitalOutputNb, Configuration, State, Mask, Polarity, &ErrorCode);
}

void WinchControl::disableBrake()
{
    uint16_t DigitalOutputNb = 4;
    uint16_t Configuration = 14;
    bool State = true;
    bool Mask = true;
    bool Polarity = 0;
    //    VCS_DigitalOutputConfiguration(keyHandle, NodeId, DigitalOutputNb, Configuration, State, Mask, Polarity, &ErrorCode);
}

int WinchControl::getPosition()
{
    long pos;
    //    VCS_GetPositionIs(keyHandle, NodeId, &pos, &ErrorCode);
    return (int) pos;
}

double WinchControl::getTetherLength()
{
    return ticks2length(getPosition());
}

int WinchControl::getVelocity()
{
    long vel;
    //    VCS_GetVelocityIs(keyHandle, NodeId, &vel, &ErrorCode);
    return (int) vel;
}

int WinchControl::getCurrent()
{
    short cur;
    //    VCS_GetCurrentIs(keyHandle, NodeId, &cur, &ErrorCode);
    return (int) cur;
}

bool WinchControl::openDevice()
{
    nodeId = 1;

    
    //    keyHandle = VCS_OpenDevice((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", (char*) "/dev/ttyS0", &ErrorCode);
    if( keyHandle == 0 )
    {
        log(Error) << "Open device failure, error code = 0x" << hex << ErrorCode << endlog();
        return false;
    }
    else
    {
        cout << "Houston, we have connection." << endlog();
    }


    int IsInFault = true;
    //    VCS_GetFaultState(keyHandle, NodeId, &IsInFault, &ErrorCode);
    if(IsInFault)
    {
      //        VCS_ClearFault(keyHandle, NodeId, &ErrorCode);
    }

    disableBrake();
    //    VCS_SetEnableState(keyHandle,NodeId,&ErrorCode);

    setPositionProfile(100,3000);
    //    VCS_SetMaxFollowingError(keyHandle, NodeId, 2e9, &ErrorCode);
    
    return true;
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

ORO_CREATE_COMPONENT( WinchControl )
