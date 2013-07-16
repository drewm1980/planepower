#include "PoseFromMarkersTypekit.hpp"

std::string PoseFromMarkersTypekit::getName()
{
	return "PoseFromMarkersTypekit";
}

bool PoseFromMarkersTypekit::loadTypes()
{
	RTT::types::Types()->addType(new PoseFromMarkersDataTypeInfo());

	return true;
}

bool PoseFromMarkersTypekit::loadConstructors()
{
	return true;
}

bool PoseFromMarkersTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( PoseFromMarkersTypekit )
