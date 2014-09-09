#include "lqrGainLoader.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

typedef uint64_t TIME_TYPE;

LqrGainLoader::LqrGainLoader(std::string name):TaskContext(name,PreOperational) 
{
	addPort("stateParameterization",portControllerParameterization);
	addPort("xss",portxss);
	addPort("xss0",portxss0);
	addPort("xss1",portxss1);
	addPort("G",portG);
	
	addProperty("R_control", pd.cp.R_control).doc("Control Penalty");
	addProperty("Q_alpha", pd.cp.Q_alpha).doc("Alpha Penalty");
	addProperty("Q_dalpha", pd.cp.Q_dalpha).doc("Dalpha Penalty");

	memset(&pd, 0, sizeof( pd ));

}

// Load in all of the precomputed data.
void LqrGainLoader::reload_gains()
{
	memset(&pd,0,sizeof(pd));
	int fd = open("precomputedData.dat",O_RDONLY);
	if(fd==-1) 
	{
		log(Error) << "Unable to load precomputedData.dat!" << endlog();
	}
	ssize_t bytes = read(fd, &pd, sizeof(pd));
	if (bytes != sizeof(pd))
	{
		log(Error) << "Wrong number of bytes read from precomputedData.dat!" << endlog();
	}
	close(fd);
}

bool LqrGainLoader::configureHook()
{
	reload_gains();
	return true;
}

bool  LqrGainLoader::startHook()
{
	return true;
}

void  LqrGainLoader::updateHook()
{
	TIME_TYPE trigger = TimeService::Instance()->getTicks();
	reload_gains();
	double elapsed = TimeService::Instance()->secondsSince( trigger );

	pd.cp.ts_trigger = trigger;
	pd.cp.ts_elapsed = elapsed;
	pd.xss.ts_trigger = trigger;
	pd.xss.ts_elapsed = elapsed;
	pd.xss0.ts_trigger = trigger;
	pd.xss0.ts_elapsed = elapsed;
	pd.xss1.ts_trigger = trigger;
	pd.xss1.ts_elapsed = elapsed;
	pd.G.ts_trigger = trigger;
	pd.G.ts_elapsed = elapsed;

	portControllerParameterization.write(pd.cp);
	portxss.write(pd.xss);
	portxss0.write(pd.xss0);
	portxss1.write(pd.xss1);
	portG.write(pd.G);

}

void  LqrGainLoader::stopHook()
{}

void  LqrGainLoader::cleanupHook()
{}

void  LqrGainLoader::errorHook()
{}

ORO_CREATE_COMPONENT( LqrGainLoader )
