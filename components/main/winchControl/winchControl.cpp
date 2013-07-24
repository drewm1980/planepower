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

	addOperation("move", &WinchControl::move,this).doc("move the motor");
	addOperation("vel", &WinchControl::vel,this).doc("move the motor");
	addOperation("setPositionProfile", &WinchControl::setPositionProfile,this).doc("Set the position profile").arg("Position profile velocity").arg("Position profile acceleration/deceleration");
	addOperation("setVelocityProfile", &WinchControl::setVelocityProfile,this).doc("Set the velocity profile").arg("Velocity profile acceleration/deceleration");
	addOperation("getPosition", &WinchControl::getPosition,this).doc("Get the current position");
	addOperation("getVelocity", &WinchControl::getVelocity,this).doc("Get the current velocity");
	addOperation("getCurrent", &WinchControl::getCurrent,this).doc("Get the current current");
	
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
		return openDevice();
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
		VCS_SetDisableState(keyHandle, NodeId, &ErrorCode);
		//VCS_CloseDevice(keyHandle, &ErrorCode);
	}

	bool WinchControl::move(int steps)
	{
		VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONPROFILE,&ErrorCode);
		VCS_MoveToPosition(keyHandle, NodeId, steps, 0, 1, &ErrorCode );
	}

	bool WinchControl::vel(int vel)
	{
		VCS_SetOperationMode(keyHandle, NodeId, E_POSITIONVELOCITY,&ErrorCode);
		VCS_MoveWithVelocity(keyHandle, NodeId, vel, &ErrorCode );
	}

	void WinchControl::setPositionProfile(int vel, int acc)
	{
		VCS_SetPositionProfile(keyHandle, NodeId, vel, acc, acc, &ErrorCode);
	}

	void WinchControl::setVelocityProfile(int acc)
	{
		VCS_SetVelocityProfile(keyHandle, NodeId, acc, acc, &ErrorCode);
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
		// This is only needed once for setup.
		int pEndOfSelection;
	/*	// Get the device name
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

		setProfile(7000, 1e12);
		VCS_SetMaxFollowingError(keyHandle, NodeId, 1e99, &ErrorCode);
	
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


}//namespace

