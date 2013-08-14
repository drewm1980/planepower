#!/usr/bin/env bash

# This script generates a new shell orocos component in an
# appropriately named subdirectory of the current directory.
if [ $# -ne 1 ]
then
  echo "Usage: $ new_component componentName"
  exit $E_BADARGS
fi

COMPONENT_NAME=$1 # i.e. sampleComponent
COMPONENT_CLASS_NAME=${COMPONENT_NAME^} # i.e. SampleComponent
COMPONENT_NAME_ALLCAPS=${COMPONENT_NAME^^} # i.e. SAMPLE_COMPONENT

DIR_NAME=$COMPONENT_NAME

echo Making a component named $COMPONENT_NAME with class name $COMPONENT_CLASS_NAME in directory $DIR_NAME...

mkdir -p $DIR_NAME

# Herefile for the component header
cat > $DIR_NAME/$COMPONENT_NAME.hpp << EOM
#ifndef __${COMPONENT_NAME_ALLCAPS}__
#define __${COMPONENT_NAME_ALLCAPS}__

#include <rtt/TaskContext.hpp>
#include <rtt/Component.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>

#include <stdint.h>

class ${COMPONENT_CLASS_NAME} : public RTT::TaskContext
{
public:
	$COMPONENT_CLASS_NAME(std::string name);
	virtual ~$COMPONENT_CLASS_NAME(){};

	virtual bool configureHook();
	virtual bool startHook();
	virtual void updateHook();
	virtual void stopHook();
	virtual void cleanupHook();
	virtual void errorHook();

};

#endif
EOM

# Herefile for the component code
cat > $DIR_NAME/$COMPONENT_NAME.cpp << EOM
#include "$COMPONENT_NAME.hpp"

#include <rtt/Logger.hpp>
#include <rtt/os/TimeService.hpp>
#include <rtt/Time.hpp>

using namespace std;
using namespace RTT;
using namespace RTT::os;

$COMPONENT_CLASS_NAME::$COMPONENT_CLASS_NAME(std::string name):TaskContext(name,PreOperational) 
{
	log(Error) << "foooo" << endlog();
}

bool $COMPONENT_CLASS_NAME::configureHook()
{
	return true;
}

bool  $COMPONENT_CLASS_NAME::startHook()
{
	return true;
}

void  $COMPONENT_CLASS_NAME::updateHook()
{
}

void  $COMPONENT_CLASS_NAME::stopHook()
{}

void  $COMPONENT_CLASS_NAME::cleanupHook()
{}

void  $COMPONENT_CLASS_NAME::errorHook()
{}

ORO_CREATE_COMPONENT( $COMPONENT_CLASS_NAME )
EOM

# Herefile for test.ops
cat > $DIR_NAME/test.ops << EOM
#!/usr/bin/env deployer

#Import libraries
import("$COMPONENT_NAME")
 
#Create components
loadComponent("$COMPONENT_NAME","$COMPONENT_CLASS_NAME")
 
$COMPONENT_NAME.configure()
$COMPONENT_NAME.start()
EOM

# Herefile for test.sh
cat > $DIR_NAME/test.sh << EOM
#!/usr/bin/env bash
deployer-gnulinux -lerror -s test.ops
EOM
chmod +x $DIR_NAME/test.sh

# Herefile for readme.txt
cat > $DIR_NAME/readme.txt << EOM
Add documentation for component $COMPONENT_NAME in here!!!
EOM

# Herefile for the Tupfile
cat > $DIR_NAME/Tupfile << EOM
include_rules
: foreach *.cpp |> !cxx_orocos |>
: *.o |> !ld_orocos |> 
EOM

read -p "Want to git add the component? (y or n)" -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]
then
    git add $DIR_NAME
fi
echo ""

read -p "Want to \"tup upd .\" the component? (y or n) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]
then
	(
	cd $DIR_NAME
	tup upd .
	)
fi

echo "Done!"


