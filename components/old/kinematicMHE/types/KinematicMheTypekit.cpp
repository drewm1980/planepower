#include "KinematicMheTypekit.hpp"

std::string KinematicMheTypekit::getName()
{
	return "KinematicMheTypekit";
}

bool KinematicMheTypekit::loadTypes()
{
	RTT::types::Types()->addType(new KinematicMheDataTypeInfo());

	return true;
}

bool KinematicMheTypekit::loadConstructors()
{
	return true;
}

bool KinematicMheTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( KinematicMheTypekit )
