#include "EncoderTypekit.hpp"

std::string EncoderTypekit::getName()
{
	return "EncoderTypekit";
}

bool EncoderTypekit::loadTypes()
{
	RTT::types::Types()->addType(new EncoderDataTypeInfo());

	return true;
}

bool EncoderTypekit::loadConstructors()
{
	return true;
}

bool EncoderTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( EncoderTypekit )
