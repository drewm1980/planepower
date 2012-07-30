#!/bin/bash

NAME=$1
UNAME=`perl -e "print ucfirst('$NAME')"`
UUNAME=`perl -e "print uc('$NAME')"`
echo "Going to create <$NAME>"

cd $NAME

echo "  Created $1"
echo "#!/bin/sh" > "run$NAME.sh"
echo "#\$MYOROCOSCOMPONENTS/usecases/common.sh">> "run$NAME.sh"
echo "  rosrun ocl deployer-gnulinux -linfo -s $NAME.ops" >> "run$NAME.sh"
chmod +x "run$NAME.sh"
echo "  made executable script $run$NAME.sh"

echo "#Import libraries" > "$NAME.ops"
echo "import(\"$NAME\")" >> "$NAME.ops"
echo " " >> "$NAME.ops"
echo "#Create components" >> "$NAME.ops"
echo "loadComponent(\"$NAME\",\"OCL::$NAME\")" >> "$NAME.ops"
echo " " >> "$NAME.ops"
echo "#Load properties" >> "$NAME.ops"
echo "loadService(\"$NAME\",\"marshalling\")" >> "$NAME.ops"
echo "$NAME.marshalling.loadProperties(\"../../../properties/$NAME/$NAME.cpf\")" >> "$NAME.ops"
echo " " >> "$NAME.ops"
echo "#Set activities" >> "$NAME.ops"
echo "setActivity(\"$NAME\",0.01,HighestPriority,ORO_SCHED_RT)" >> "$NAME.ops"
echo " " >> "$NAME.ops"
echo "$NAME.configure()" >> "$NAME.ops"
echo "$NAME.start()" >> "$NAME.ops"
echo "  made orocos script file $NAME.ops"

if [[ -z `grep ocl manifest.xml` ]]; then
head -n -3 manifest.xml > tmp
mv tmp manifest.xml
echo  '<depend package="rtt_ros_integration" />' >> manifest.xml
echo  '<depend package="rtt_ros_integration_std_msgs" />' >> manifest.xml
echo  '<depend package="rtt_ros_param" />' >> manifest.xml
echo  '<depend package="rtt" />' >> manifest.xml
echo  '<depend package="ocl" />' >> manifest.xml
echo '</package>' >> manifest.xml
fi

echo "  Added typical orocos dependecies to mainfest.xml"
echo "Done"
