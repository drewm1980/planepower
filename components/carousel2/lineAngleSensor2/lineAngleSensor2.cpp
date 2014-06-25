#include "lineAngleSensor2.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

#include <unistd.h>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

LineAngleSensor2::LineAngleSensor2(std::string name):TaskContext(name,PreOperational) 
{
	addPort("data", portData).doc("The output data port");
	addProperty("DownstreamHostPort", hostPortDownstream)
                .doc("Downstream port to listen on (1024-65535 inclusive)");    
}

bool LineAngleSensor2::configureHook()
{
	memset(&lineAngles, 0, sizeof( LineAngles ));
	return true;
}

bool  LineAngleSensor2::startHook()
{
	// Trigger updateHook to be called at least once
	keepRunning = true;
	this->getActivity()->trigger(); 
	return true;

	// initialize UDP_Communication
	err = openUDPServerSocket(&udp_server, hostPortDownstream, UDP_SOCKET_TIMEOUT);

        if (err != UDP_ERR_NONE)
        {
                printStatus();
                return false;
        }
        
        // Initialize message decoding
        init_decoding();
	return true;
}

void  LineAngleSensor2::updateHook()
{

	TIME_TYPE trigger = TimeService::Instance()->getTicks();

	// Do blocking wait for new line angle measurements here
	usleep(50000); // make sure we don't burn spin the cpu in a high priority thread during testing.

	int err;
	int recv_len;
        size_t data_len = sizeof(input_stream);
        err = receiveUDPServerData(&udp_server, (void *)&input_stream, data_len, &recv_len);

        if (recv_len != data_len)
        {
                printf("Wrong number of bytes in received UDP packet!\n");
                printf("Expected %lu bytes, Received %d bytes!\n",data_len,recv_len);
                err = UDP_ERR_RECV;
        }

	if ( err != UDP_ERR_NONE)
        {
                exception();
        } else {
		//gettimeofday(&tv_now,NULL); //timestamp from receiving to calculate latency
	//2. decode data                
	        err  = data_decode(input_stream);
		if (err != DEC_ERR_NONE)
                	exception();

                if(err==DEC_ERR_NONE){

	                switch_read_write(); //only switch read write if data decoding was succesfull
                        Data* data = get_read_pointer();

                        if(input_stream[3]==LINE_ANGLE_ID){
     	                        // Send a character (to gpio of arduino)
                                // to stop the arduino-timer
                                /*
                                FILE *myFile;
        	                myFile = fopen("/dev/ttyUSB0", "w");
                                fputs ("a", myFile);
                                fclose (myFile);
                                */
			

                        	printf("LINE_ANGLE_ID content:");
                                // print_mem((void *)&data->bone_arm.line_angle,sizeof(LINE_ANGLE));

                                printf("Azimuth %i\n",data->bone_arm.line_angle.azimuth_raw);
                                printf("Elevation %i\n",data->bone_arm.line_angle.elevation_raw);
                                printf("\n");
                                }

                 }else{
                 	printf("UNKNOW PACKAGE with id %d\n",input_stream[3]);
                        exit(1);
                 }
	}







	//lineAngles.azimuth = -0.5 + 1.2 * sin(.2*3.1415 * trigger + 1.8);
	//lineAngles.elevation = -0.4 + 0.7 * sin(.1*3.1415 * trigger + 1.8);

	lineAngles.ts_trigger = trigger;
	lineAngles.ts_elapsed = TimeService::Instance()->secondsSince( trigger );
	portData.write(lineAngles);

	// Re-trigger this comonent internally
	if(keepRunning) this->getActivity()->trigger(); 
}

void  LineAngleSensor2::stopHook()
{
	keepRunning = false;
	closeUDPServerSocket( &udp_server );
	printStatus();

}

void  LineAngleSensor2::cleanupHook()
{}

void  LineAngleSensor2::errorHook()
{}

void LineAngleSensor2::printStatus()
{
        Logger::In in( getName() );

        switch( err )
        {
        case UDP_ERR_NONE:
                break;
        case  UDP_ERR_INET_ATON:
                log( Error ) << "Failed decoding IP address" << endlog();
                break;

        case UDP_ERR_SEND:
                log( Error ) << "Failed sending UDP data" << endlog();
                break;

        case UDP_ERR_CLOSE_SOCKET:
                log( Error ) << "Failed closing UDP socket" << endlog();
                break;

        case UDP_ERR_OPEN_SOCKET:
                log( Error ) << "Failed inserting UDP socket" << endlog();
                break;

        case UDP_ERR_BIND_SOCKET_PORT:
                log( Error ) << "Failed binding port to socket" << endlog();
                break;

        case UDP_ERR_RECV:
                log( Error ) << "Failed receiving UDP data" << endlog();
                break;
default:
                log( Error ) << "Undefined UDP error" << endlog();
                break;
        }

        switch( err )
        {
        case DEC_ERR_NONE:
                break;

        case  DEC_ERR_START_BYTE:
                log( Error ) << "Start byte is not 0x99" << endlog();
                break;

        case DEC_ERR_CHECKSUM:
                log( Error ) << "Wrong checksum" << endlog();
                break;

        case DEC_ERR_UNKNOWN_BONE_PACKAGE:
                log( Error ) << "Received unknown package from beaglebone" << endlog();
                break;

        case DEC_ERR_UNKNOWN_LISA_PACKAGE:
                log( Error ) << "Received unknown package from lisa" << endlog();
                break;

        case DEC_ERR_UNKNOWN_SENDER:
                log( Error ) << "Received package from unknown sender" << endlog();
                break;

        default:
                log( Error ) << "Undefined decoding error" << endlog();
	}

}


ORO_CREATE_COMPONENT( LineAngleSensor2 )
