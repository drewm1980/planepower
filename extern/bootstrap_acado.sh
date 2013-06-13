echo "Bootstrapping ACADO..."
sudo apt-get install cmake

(
mkdir -p acado/build
cd acado/build
cmake -DACADO_INTERNAL:BOOL=ON -DACADO_WITH_EXAMPLES:BOOL=OFF ..
make -j3
)

