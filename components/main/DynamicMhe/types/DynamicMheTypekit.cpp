#include "DynamicMheTypekit.hpp"

std::string DynamicMheTypekit::getName()
{
	return "DynamicMheTypekit";
}

bool DynamicMheTypekit::loadTypes()
{
	RTT::types::Types()->addType(new DynamicMheStateEstimateInfo());
	RTT::types::Types()->addType(new DynamicMheHorizonInfo());

	return true;
}

bool DynamicMheTypekit::loadConstructors()
{
	return true;
}

bool DynamicMheTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( DynamicMheTypekit )
