#include "IndoorsCarouselSimulatorTypekit.hpp"

std::string IndoorsCarouselSimulatorTypekit::getName()
{
	return "IndoorsCarouselSimulatorTypekit";
}

bool IndoorsCarouselSimulatorTypekit::loadTypes()
{
	RTT::types::Types()->addType(new IndoorsCarouselSimulatorDataTypeInfo());

	return true;
}

bool IndoorsCarouselSimulatorTypekit::loadConstructors()
{
	return true;
}

bool IndoorsCarouselSimulatorTypekit::loadOperators()
{
	return true;
}

ORO_TYPEKIT_PLUGIN( IndoorsCarouselSimulatorTypekit )
