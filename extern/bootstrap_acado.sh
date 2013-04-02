echo "Bootstrapping ACADO..."
(
mkdir -p acado/build
cd acado/build
cmake -DACADO_INTERNAL:BOOL=ON -DWITH_ACADO_EXAMPLES:BOOL=OFF ..
make -j3
)

