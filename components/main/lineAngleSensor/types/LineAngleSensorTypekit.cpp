#include "LineAngleSensorTypekit.hpp"

std::string LineAngleSensorTypekit::getName()
{
	return "LineAngleSensorTypekit";
}

bool LineAngleSensorTypekit::loadTypes()
{
	RTT::types::Types()->addType(new LineAngleSensorDataTypeInfo());

	return true;
}

bool LineAngleSensorTypekit::loadConstructors()
{
	return true;
}

bool LineAngleSensorTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( LineAngleSensorTypekit )
