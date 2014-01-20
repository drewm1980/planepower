#include "WinchControlTypekit.hpp"

std::string WinchControlTypekit::getName()
{
	return "WinchControlTypekit";
}

bool WinchControlTypekit::loadTypes()
{
	RTT::types::Types()->addType(new WinchControlDataTypeInfo());

	return true;
}

bool WinchControlTypekit::loadConstructors()
{
	return true;
}

bool WinchControlTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( WinchControlTypekit )
