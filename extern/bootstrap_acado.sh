echo "Bootstrapping ACADO..."

#echo "Downloading and unpacking a stable acado revision from tar file..."
#ACADO_PRIVATE=acado_private_r3094_trimmed
#if [ ! -f $ACADO_PRIVATE.tar.gz ]; then
	#scp gitmirrorbot@moinette.esat.kuleuven.be:$ACADO_PRIVATE.tgz .
#fi
#[ ! -d $ACADO_PRIVATE ] && tar -xvf $ACADO_PRIVATE.tgz

ACADO_PUBLIC=acado
#echo "Cloning acado_public"
#git clone gitmirrorbot@moinette.esat.kuleuven.be:acado_public $ACADO_PUBLIC
#(
#cd $ACADO_PUBLIC
#git co kurt_at_planepower_during_embocon
#)
echo "Making acado_public..."
(
mkdir -p $ACADO_PUBLIC/build
cd $ACADO_PUBLIC/build
cmake -DWITH_ACADO_EXAMPLES=OFF ..
make
)

ACADO_PRIVATE=acado_private
#echo "Cloning acado_private"
#git clone gitmirrorbot@moinette.esat.kuleuven.be:acado_private $ACADO_PRIVATE
#git co kurt_at_planepower_during_embocon
#(
#cd $ACADO_PRIVATE
#git co kurt_at_planepower_during_embocon
#)
#echo "Making acado_private..."
#(
#mkdir -p $ACADO_PRIVATE/build
#cd $ACADO_PRIVATE/build
#cmake ..
#make
#)

source $ACADO_PUBLIC/build/acado_env.sh
#source $ACADO_PRIVATE/build/acado_env.sh

echo "Make mhe codegen stuff..."
(
cd $ACADO_PRIVATE/testing/mvukov/mhe_export
cmake .
make
)

