#include "LEDTrackerTypekit.hpp"

std::string LEDTrackerTypekit::getName()
{
	return "LEDTrackerTypekit";
}

bool LEDTrackerTypekit::loadTypes()
{
	RTT::types::Types()->addType(new LEDTrackerDataTypeInfo());

	return true;
}

bool LEDTrackerTypekit::loadConstructors()
{
	return true;
}

bool LEDTrackerTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( LEDTrackerTypekit )
