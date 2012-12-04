echo "Bootstrapping OROCOS..."
(
cd rtt
cmake . -DENABLE_CORBA=ON -DCORBA_IMPLEMENTATION=OMNIORB
make -j4
sudo make install
)
(
cd log4cpp
cmake .
make -j4
sudo make install
)
(
cd ocl
cmake .
make -j4
sudo make install
)
(
cd utilmm
cmake .
make -j4
sudo make install
)
(
cd typelib
cmake .
make -j4
sudo make install
)
