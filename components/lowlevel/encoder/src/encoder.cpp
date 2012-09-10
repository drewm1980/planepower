#include "encoder.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::Encoder)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

// Gear ratio of the gearbox of the carousel
#define GEAR_RATIO 32.8
// Number of pulses per revolution of the encoder
#define PULSES_PER_REVOLUTION 4096.0
// Encoder will overflow at this value
#define ENCODER_OVERFLOW 2147483648.0
// Encoder threshold to detect overflow
#define ENCODER_TRESHOLD ENCODER_OVERFLOW/10.0
// Pi
#define PI 3.1415926535

namespace OCL
{
     Encoder::Encoder(std::string name)
         : TaskContext(name)
     {
        // Add properties

	addProperty("encoderPort",encoderPort);
	addProperty("extra_ticks",extra_ticks);

	// Add ports

        addPort( "delta",_delta ).doc("The position of the carousel");
        addPort( "omega",_omega ).doc("The rotation speed of the carousel");
	addEventPort("encoderTimeStampIn",_encoderTimeStampIn).doc("time stamp of the encoder signal");
	addPort("encoderTimeStampOut",_encoderTimeStampOut).doc("time stamp of the encoder signal");
	addPort("computationTime",_computationTime);
	addPort("deltaOmega",_deltaOmega);

	deltaOmega.resize(2,0.0);
	encoderTimeStamp = RTT::os::TimeService::Instance()->getTicks(); // Get current time

	_delta.setDataSample( 0.0 );
	_delta.write( 0.0 );
	_omega.setDataSample( 0.0 );
	_omega.write( 0.0 );
	_encoderTimeStampOut.setDataSample( encoderTimeStamp );
	_encoderTimeStampOut.write( encoderTimeStamp );
	_computationTime.setDataSample( encoderTimeStamp );
	_computationTime.write( encoderTimeStamp );
	_deltaOmega.setDataSample( deltaOmega );
	_deltaOmega.write( deltaOmega );
    }
    

    Encoder::~Encoder()
    {
    }

    bool  Encoder::configureHook()
    {
	assert(this->hasPeer("soemMaster"));
	assert(this->getPeer("soemMaster")->provides()->hasService("Slave_1001"));
	assert(this->getPeer("soemMaster")->provides()->getService("Slave_1001")->provides()->hasOperation("readEncoder"));
	readEncoder = getPeer("soemMaster")->provides()->getService("Slave_1001")->provides()->getOperation("readEncoder");
        return true;
     }

    bool  Encoder::startHook()
    {
	for (int i=0;i<REGSIZE;i++) diffcount[i]=0;
	for (int i=0;i<REGSIZE;i++)  difftime[i]=0;
	regi = 0;
	mycount_new = readEncoder(encoderPort) ; // Read once, because the first time it seems to fail and give back some random value
	mycount_new=0;
	mycount_old=0;
	tickRegister = RTT::os::TimeService::Instance()->getTicks(); // Get current time and store it in the register
	diff = 0.0;
	extra_ticks = 0.0;
	overflowup = false;
	overflowdown = false;
        return true;
    }

    void  Encoder::updateHook()
    {
RTT::os::TimeService::ticks tic = RTT::os::TimeService::Instance()->getTicks();
	mycount_new = readEncoder(encoderPort) ;

#if 0
	SendHandle<int(unsigned int)> encoderHandle = readEncoder.send(encoderPort);

	// collect takes the return value of getName() as first argument and fills it in:
	SendStatus ss = encoderHandle.collect(mycount_new); // blocks until operation is complete
	if (!(ss == SendSuccess)) {
		//cout << "mycount_new was: " << mycount_new << endl; 
	}
#endif
	_computationTime.write((TIME_TYPE) RTT::os::TimeService::Instance()->getTicks(tic) );
	_encoderTimeStampIn.read(encoderTimeStamp);

	
	// First calculate the time passed from previous updateHook
	myticks = RTT::os::TimeService::Instance()->getTicks(); // Get current time
	deltansec = RTT::os::TimeService::ticks2nsecs(myticks-tickRegister); // Calculate the difference with previous time (in nanoseconds)
	myperiod = deltansec/1.0e9; // Convert to seconds
	if (myperiod==0.0) return;
	tickRegister=myticks; // Store time to be used in next updateHook
	// Now calculate the difference in encoder ticks
	diff=mycount_new-mycount_old; // Calculate the difference with previous encoder ticks
	// Try to catch if the encoder has encountered an overflow
	if(abs(diff>ENCODER_TRESHOLD)){ //Overflow!!
		// Ok, so we have an overflow. Now detect in which direction, and make it correct.
//		cout << "Encoder overflow detected" << endl;
//cout << "mycount_old " << mycount_old << endl;
//cout << "mycount_new " << mycount_new << endl;
		if(mycount_old > 0){diff+=ENCODER_OVERFLOW; overflowup = true;}
		if(mycount_old < 0){diff-=ENCODER_OVERFLOW; overflowdown=true;}
	}
	mycount_old=mycount_new; // And store previous encoder ticks

	// And calculate the velocity
	diffcount[regi] = diff;
	difftime[regi] = myperiod;
	regi=(regi+1) % REGSIZE;

	//Average out over [REGSIZE] measurements
	omega=0;
	for (int i=0;i<REGSIZE;i++){
		omega += (double) diffcount[i]/difftime[i];
	}
	omega /= REGSIZE;

	omega*=2*PI/GEAR_RATIO/PULSES_PER_REVOLUTION; //Convert to correct dimensions (rad/s)

	// Now calculate the angle of the carousel

	// First, account for overflow situations
	if(overflowup){
		extra_ticks += ENCODER_OVERFLOW; // This many ticks will be ignored, so we add them
		extra_ticks = fmod(extra_ticks,GEAR_RATIO*PULSES_PER_REVOLUTION); // To be sure that extra_ticks doesn't become too big (probably never happens, or after 4 years of running, but better safe than sorry :-) )
		overflowup=false; // Put the flag back to false, because it isn't true anymore (or we have to prove otherwise in next update)
	}
	if(overflowdown){
		extra_ticks -= ENCODER_OVERFLOW; // This many ticks will be ignored, so we add them
		extra_ticks = fmod(extra_ticks,GEAR_RATIO*PULSES_PER_REVOLUTION); // To be sure that extra_ticks doesn't become too big (probably never happens, or after 4 years of running, but better safe than sorry :-) )
		overflowdown=false; // Put the flag back to false, because it isn't true anymore (or we have to prove otherwise in next update)
	}
	// Now calculate the angle
	delta = 2.0*PI*( (double) mycount_new + extra_ticks)/(GEAR_RATIO*PULSES_PER_REVOLUTION);
	// And put it on the ports
	deltaOmega[0] = -delta;
	deltaOmega[1] = -omega;
	_encoderTimeStampOut.write(encoderTimeStamp);
	_delta.write(-delta); // with a minus sign because it is defined in counter clock wise direction
	_omega.write(-omega); // Put it on the port. With minus sign, because rotation should be positive for counter clock wise rotation
	_deltaOmega.write(deltaOmega);
    }

    void  Encoder::stopHook()
    {
    }

    void  Encoder::cleanUpHook()
    {
    }


}//namespace

