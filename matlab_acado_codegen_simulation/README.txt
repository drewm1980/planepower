README

1. Update the SVN

2. Recompile the ACADO and MHE exort using instructions from one of my previous emails

3. Outside MATLAB, from terminal, type:

mkdir build
cd build
cmake ..
make

CMake should configure and generate the Makefiles. If not, return to 2. and possibly do not call me. :D

4. From now on, MATLAB script will be in charge of calling the makefiles. Go to MATLAB and call MPC_MHE.m

****
In case CMake complains once you are able to proceed to 4. and run .m script successfully at least once,
go to terminal and type

cd build
cmake ..
make

And then you can return to 4.
****


CU,
Milan
