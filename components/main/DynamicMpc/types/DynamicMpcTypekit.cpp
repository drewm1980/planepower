#include "DynamicMpcTypekit.hpp"

std::string DynamicMpcTypekit::getName()
{
	return "DynamicMpcTypekit";
}

bool DynamicMpcTypekit::loadTypes()
{
	RTT::types::Types()->addType(new DynamicMpcHorizonInfo());

	return true;
}

bool DynamicMpcTypekit::loadConstructors()
{
	return true;
}

bool DynamicMpcTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( DynamicMpcTypekit )
