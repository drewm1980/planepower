#include <rtt/types/TypekitPlugin.hpp>
#include <rtt/types/SequenceTypeInfo.hpp>
#include <rtt/types/StructTypeInfo.hpp>

#include <rtt/typekit/StdTypeInfo.hpp>
#include <rtt/types/TemplateConstructor.hpp>
#include <rtt/types/Operators.hpp>
#include <rtt/types/OperatorTypes.hpp>
#include <rtt/internal/mystd.hpp>

#include <rtt/typekit/StdTypeInfo.hpp>

#include <string>
#include <vector>
#include <stdint.h>

using namespace RTT;
using namespace RTT::detail;
using namespace std;

class HighwindTypes
	: public types::TypekitPlugin
{
public:
	
	bool loadTypes()
	{
		// std::vector< float >
		types::Types()->addType( new types::SequenceTypeInfo<vector< float > >( "floats" ) );

		// uint64_t & int64_t
		types::Types()->addType(new StdTypeInfo< uint64_t >( "uint64_t" ));
		types::Types()->addType(new StdTypeInfo< int64_t >( "int64_t" ));

		return true;
	}

	bool loadOperators()
	{ return true; }
	
	bool loadConstructors()
	{ return true; }
	
	std::string getName()
	{
		return "HighwindTypes";
	}
};

ORO_TYPEKIT_PLUGIN( HighwindTypes )
