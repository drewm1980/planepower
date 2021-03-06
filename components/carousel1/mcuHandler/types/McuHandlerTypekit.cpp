#include "McuHandlerTypekit.hpp"

std::string McuHandlerTypekit::getName()
{
	return "McuHandlerTypekit";
}

bool McuHandlerTypekit::loadTypes()
{
	RTT::types::Types()->addType(new McuHandlerDataTypeInfo());
	RTT::types::Types()->addType(new ControlSurfacesValuesTypeInfo());

	return true;
}

bool McuHandlerTypekit::loadConstructors()
{
	return true;
}

bool McuHandlerTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( McuHandlerTypekit )
