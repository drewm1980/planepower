#include "proportionalController.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

ProportionalController::ProportionalController(std::string name):TaskContext(name,PreOperational) 
{
	log(Error) << "foooo" << endlog();
}

bool ProportionalController::configureHook()
{
	return true;
}

bool  ProportionalController::startHook()
{
	return true;
}

void  ProportionalController::updateHook()
{
}

void  ProportionalController::stopHook()
{}

void  ProportionalController::cleanupHook()
{}

void  ProportionalController::errorHook()
{}

ORO_CREATE_COMPONENT( ProportionalController )
