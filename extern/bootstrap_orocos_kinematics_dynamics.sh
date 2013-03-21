(
echo "Bootstrapping orocos_kinematics_dynamics..."
cd orocos_kinematics_dynamics/orocos_kdl
mkdir -p build
cd build
cmake ..
make
make check
sudo make install
echo "Done bootstrapping orocos_kinematics_dynamics."
)
