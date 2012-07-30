#include "segFaulter.hpp"
#include <ocl/Component.hpp>
#include <signal.h>

ORO_CREATE_COMPONENT( OCL::SegFaulter)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
	SegFaulter::SegFaulter(std::string name) : TaskContext(name) { }
	SegFaulter::~SegFaulter() { } 
	bool  SegFaulter::configureHook()
	{
		countDown = 10;
		return true;
	}

	bool  SegFaulter::startHook() { return true; }

	void  SegFaulter::updateHook()
	{
		if(--countDown==0)
		{
			cout << "(segFaulter): KAAAATSO!!!" << endl;
			cout << "(segFaulter): KAAAATSO!!!" << endl;
			cout << "(segFaulter): KAAAATSO!!!" << endl;
			cout << "(segFaunter): If segfaulting fucked up your terminal, type <enter>reset<enter>" << endl;
			raise(SIGSEGV);	 // All the fun of crashing, without the danger of memory corruption!
		}
		else
		{
			cout << "(segFaulter): This carousel will self destruct in " << countDown << endl;
		}
	}

	void  SegFaulter::stopHook() { } 
	void  SegFaulter::cleanUpHook() { }
}//namespace

