#include "winchControl.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::WinchControl)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 WinchControl::WinchControl(std::string name)
		 : TaskContext(name)
	 {
		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

	// Add ports

		ports()->addPort( "inPort",_inPort ).doc("The description of the port");
		ports()->addPort( "outPort",_outPort ).doc("The description of the port");

	// Add operations

//	addOperation("setRelativePosition", &WinchControl::setRelativePosition,this).doc("Move the motor relative to its current position");
//	addOperation("setAbsolutePosition", &WinchControl::setAbsolutePosition,this).doc("Move the motor to this position");
//	addOperation("setVelocity", &WinchControl::setVelocity,this).doc("Let the motor move with this velocity");
	addOperation("changeTetherLength", &WinchControl::changeTetherLength,this).doc("Change the tether length relatively, with input in meter. Positive means extending tether");
	addOperation("setTetherLength", &WinchControl::setTetherLength,this).doc("Change the tether length absolute, with input in meter. This is an absolute value!");
	addOperation("setPositionProfile", &WinchControl::setPositionProfile,this).doc("Set the position profile, first max velocity in rpm???, second max acceleration in rpm^2??");//.arg("Position profile velocity").arg("Position profile acceleration/deceleration");
	addOperation("setCurrentTetherLength", &WinchControl::setCurrentTetherLength,this).doc("Specify the current tether length. This means changing the current tetherlength in software, but not in hardware.");//.arg("Position profile velocity").arg("Position profile acceleration/deceleration");
	addOperation("setVelocityProfile", &WinchControl::setVelocityProfile,this).doc("Set the velocity profile, max acceleration in rpm^2??");//.arg("Velocity profile acceleration/deceleration");
	addOperation("getPosition", &WinchControl::getPosition,this).doc("Get the current position in ticks");
	addOperation("getTetherLength", &WinchControl::getTetherLength,this).doc("Get the current tether length in meter");
	addOperation("getVelocity", &WinchControl::getVelocity,this).doc("Get the current velocity, in rpm?");
	addOperation("getCurrent", &WinchControl::getCurrent,this).doc("Get the current current");
	addOperation("enableBrake", &WinchControl::enableBrake,this).doc("Enable the brake");
	addOperation("disableBrake", &WinchControl::disableBrake,this).doc("Disable the brake");
	
	NodeId = 0;

	}
	

	WinchControl::~WinchControl()
	{
	}

	bool  WinchControl::configureHook()
	{
		return true;
	 }

	bool  WinchControl::startHook()
	{
		if(!openDevice()){return false;}
		double tether_length_ticks;
		if( loadTetherLength(tether_length_ticks) ){
			cout << "My current tether length, based on previously stored tether length ticks, is " << ticks2length(tether_length_ticks) << " m" << endl;
		}
		VCS_DefinePosition(keyHandle, NodeId, tether_length_ticks,&ErrorCode);
		return true;
	}

	void  WinchControl::updateHook()
	{
	}

	void  WinchControl::stopHook()
	{
		int IsInFault;
		VCS_GetFaultState(keyHandle, NodeId, &IsInFault, &ErrorCode);
		if(IsInFault){
			VCS_ClearFault(keyHandle, NodeId, &ErrorCode);
		}
		enableBrake();
		saveTetherLength((int) getPosition());
		VCS_SetDisableState(keyHandle, NodeId, &ErrorCode);
		//VCS_CloseDevice(keyHandle, &ErrorCode);
	}


	bool WinchControl::setRelativePosition(int steps)
	{
		if(ticks2length(steps+getPosition()) < MIN_TETHER_LENGTH || ticks2length(steps+getPosition()) > MAX_TETHER_LENGTH){
			cout << "Your move is invalid. Tether length should be between " << MIN_TETHER_LENGTH << " and " << MAX_TETHER_LENGTH << " m" << endl;
			return false;
		}
		if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONPROFILE,&ErrorCode) ){
			return false;
		}
		if( !VCS_MoveToPosition(keyHandle, NodeId, steps, 0, 1, &ErrorCode ) ){
			return false;
		}
		return true;
	}

	bool WinchControl::setAbsolutePosition(int steps)
	{
		if(ticks2length(steps) < MIN_TETHER_LENGTH || ticks2length(steps) > MAX_TETHER_LENGTH){
			cout << "Your move is invalid. Tether length should be between " << MIN_TETHER_LENGTH << " and " << MAX_TETHER_LENGTH << " m" << endl;
			return false;
		}
		if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONPROFILE,&ErrorCode) ){
			return false;
		}
		if( !VCS_MoveToPosition(keyHandle, NodeId, steps, 1, 1, &ErrorCode ) ){
			return false;
		}
		return true;
	}

	bool WinchControl::setVelocity(int vel)
	{
		if( !VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONVELOCITY,&ErrorCode) ){
			return false;
		}
		if( !VCS_MoveWithVelocity(keyHandle, NodeId, vel, &ErrorCode ) ){
			return false;
		}
		return true;
	}

	bool WinchControl::changeTetherLength(double length){
		double ticks = length2ticks(length);
		return setRelativePosition((int) ticks);
	}

	bool WinchControl::setTetherLength(double length){
		double ticks = length2ticks(length);
		return setAbsolutePosition((int) ticks);
	}	

	double WinchControl::ticks2length(int ticks){
		return ((double) ticks)/((double) WINCH_SCALING);
	}

	int WinchControl::length2ticks(double length){
		double ticks = length*((double) WINCH_SCALING);
		return (int) ticks;
	}

	void WinchControl::setPositionProfile(int vel, int acc)
	{
		VCS_SetPositionProfile(keyHandle, NodeId, vel, acc, acc, &ErrorCode);
	}

	void WinchControl::setVelocityProfile(int acc)
	{
		VCS_SetVelocityProfile(keyHandle, NodeId, acc, acc, &ErrorCode);
	}

	void WinchControl::setCurrentTetherLength(double length){
	        int tether_length_ticks = length2ticks(length);
                VCS_DefinePosition(keyHandle, NodeId, tether_length_ticks,&ErrorCode);
	}

	void WinchControl::enableBrake()
	{
		WORD DigitalOutputNb = 4;
		WORD Configuration = 14;
		bool State = false;
		bool Mask = true;
		bool Polarity = 0;
		VCS_DigitalOutputConfiguration(keyHandle, NodeId, DigitalOutputNb, Configuration, State, Mask, Polarity, &ErrorCode);
	}

	void WinchControl::disableBrake()
	{
		WORD DigitalOutputNb = 4;
		WORD Configuration = 14;
		bool State = true;
		bool Mask = true;
		bool Polarity = 0;
		VCS_DigitalOutputConfiguration(keyHandle, NodeId, DigitalOutputNb, Configuration, State, Mask, Polarity, &ErrorCode);
	}

	int WinchControl::getPosition()
	{
		long pos;
		VCS_GetPositionIs(keyHandle, NodeId, &pos, &ErrorCode);
		return (int) pos;
	}

	double WinchControl::getTetherLength(){
		return ticks2length(getPosition());
	}

	int WinchControl::getVelocity()
	{
		long vel;
		VCS_GetVelocityIs(keyHandle, NodeId, &vel, &ErrorCode);
		return (int) vel;
	}

	int WinchControl::getCurrent()
	{
		short cur;
		VCS_GetCurrentIs(keyHandle, NodeId, &cur, &ErrorCode);
		return (int) cur;
	}

	bool WinchControl::openDevice(){
		nodeId = 1;
	/*	// This is only needed once for setup.
		int pEndOfSelection;
		// Get the device name
		char pDeviceNameSel[100];
		if( !VCS_GetDeviceNameSelection(true, (char*) &pDeviceNameSel, 90, &pEndOfSelection, &ErrorCode) )
		{
			log(Error) << "Get device name failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
		cout << "DeviceName: " << pDeviceNameSel << endl;
		while(!pEndOfSelection){
			if( !VCS_GetDeviceNameSelection(false, (char*) &pDeviceNameSel, 90, &pEndOfSelection, &ErrorCode) )
			{
				log(Error) << "Get device name failure, error code = 0x" << hex << ErrorCode << endlog();
				return false;
			}
			cout << "DeviceName: " << pDeviceNameSel << endl;
		}
	
		// Get the protocol stack name
		char pProtocolNameSel[100];
		if( !VCS_GetProtocolStackNameSelection((char*) "EPOS2", true, (char*) &pProtocolNameSel, 90, &pEndOfSelection, &ErrorCode) )
		{
			log(Error) << "Get protocol name failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
		cout << "Protocol stack name: " << pProtocolNameSel << endl;
		while(!pEndOfSelection){
			if( !VCS_GetProtocolStackNameSelection((char*) "EPOS2", false, (char*) &pProtocolNameSel, 90, &pEndOfSelection, &ErrorCode) )
			{
				log(Error) << "Get protocol name failure, error code = 0x" << hex << ErrorCode << endlog();
				return false;
			}
			cout << "Protocol stack name: " << pProtocolNameSel << endl;
		}
	
	
		// Get the interface name
		char pInterfaceNameSel[100];
		if( !VCS_GetInterfaceNameSelection((char*) "EPOS2", (char*) "MAXON_RS232", true, (char*) &pInterfaceNameSel, 90, &pEndOfSelection, &ErrorCode) )
		{
			log(Error) << "Get interface name failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
		cout << "Interface name: " << pInterfaceNameSel << endl;
		while(!pEndOfSelection){
			if( !VCS_GetInterfaceNameSelection((char*) "EPOS2", (char*) "MAXON_RS232", false, (char*) &pInterfaceNameSel, 90, &pEndOfSelection, &ErrorCode) )
			{
				log(Error) << "Get interface name failure, error code = 0x" << hex << ErrorCode << endlog();
				return false;
			}
			cout << "Interface name: " << pInterfaceNameSel << endl;
		}
	
		// Get the port name
		char pPortSel[100];
		bool PortName = VCS_GetPortNameSelection((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", true, (char*) &pPortSel, 90, &pEndOfSelection, &ErrorCode);
		cout << "Port name: " << pPortSel << endl;
		while(!pEndOfSelection){
			PortName = VCS_GetPortNameSelection((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", false, (char*) &pPortSel, 90, &pEndOfSelection, &ErrorCode);
			cout << "Port name: " << pPortSel << endl;
		}
		if( PortName == 0 )
		{
			log(Error) << "Get port name failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
	
	
		// Get Baud rate selection
		long unsigned int pBaudrateSel;
		bool Baudrate = VCS_GetBaudrateSelection((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", (char*) "/dev/ttyS0", true, &pBaudrateSel, &pEndOfSelection, &ErrorCode);
		cout << "Baud rate: " << pBaudrateSel << " Bit/s" << endl;
		while(!pEndOfSelection){
			Baudrate = VCS_GetBaudrateSelection((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", (char*) "/dev/ttyS0", false, &pBaudrateSel, &pEndOfSelection, &ErrorCode);
			cout << "Baud rate: " << pBaudrateSel << " Bit/s" << endl;
		}
		if( Baudrate == 0 )
		{
			log(Error) << "Get Baudrate failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
	*/	
		// Start with opening the device
		keyHandle = VCS_OpenDevice((char*) "EPOS2", (char*) "MAXON_RS232", (char*) "RS232", (char*) "/dev/ttyS0", &ErrorCode);
		if( keyHandle == 0 )
		{
			log(Error) << "Open device failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
		else{cout << "Houston, we have connection." << endlog();}


		int IsInFault;
		VCS_GetFaultState(keyHandle, NodeId, &IsInFault, &ErrorCode);
		if(IsInFault){
			VCS_ClearFault(keyHandle, NodeId, &ErrorCode);
		}

		disableBrake();
		VCS_SetEnableState(keyHandle,NodeId,&ErrorCode);

		setPositionProfile(100,3000);
		VCS_SetMaxFollowingError(keyHandle, NodeId, 2e9, &ErrorCode);
	
		// Initialise the new baud rate and timeout
/*
		if( !VCS_SetProtocolStackSettings( keyHandle, 115200, 10000, &ErrorCode) )
		{
			log(Error) << "Set protocol stack settings failed!, error code = 0x" << hex << ErrorCode << endlog();
		return false;
		}
	
		int clearFault = VCS_ClearFault(keyHandle, 1, &ErrorCode);
		if( ErrorCode != 0 )
		{
			log(Error) << "ClearFault failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
	
		// Get Motor type
		short unsigned int pMotorType;
		int MotorType = VCS_GetMotorType(keyHandle, 1, &pMotorType, &ErrorCode);
		cout << "MotorType: " << pMotorType << endl;
		ErrorCode &= 0xFFFFFFFF;
		if( !MotorType )
		{
			log(Error) << "Get MotorType failure, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
	
		int IsInFault;
		nodeId = 0;
		while( !VCS_GetFaultState(keyHandle, nodeId, &IsInFault, &ErrorCode) )
		{
			cout << nodeId << endl;
			log(Error) << "Get fault state failed!, error code = 0x" << hex << ErrorCode << endlog();
			//return false;
			nodeId++;
		}
		cout << "IsInFault: " << IsInFault << endl;
	
		if( !VCS_SetMotorType(keyHandle, nodeId, 10, &ErrorCode) )
		{
			log(Error) << "Set motor type failed!, error code = 0x" << hex << ErrorCode << endlog();
			return false;
		}
*/
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

}//namespace

