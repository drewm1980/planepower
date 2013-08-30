#include "LisaBboneClientTypekit.hpp"

std::string LisaBboneClientTypekit::getName()
{
	return "LisaBboneClientTypekit";
}

bool LisaBboneClientTypekit::loadTypes()
{
	RTT::types::Types()->addType(new LisaBboneClientDataTypeInfo());

	return true;
}

bool LisaBboneClientTypekit::loadConstructors()
{
	return true;
}

bool LisaBboneClientTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( LisaBboneClientTypekit )
