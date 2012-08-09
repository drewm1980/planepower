#!/usr/bin/env bash

. ../../../../matlab_acado_codegen_simulation/mpc_settings.m

../nmpc_codegen $W0 $Ncvp $Tc 1 $r
  
sed -i "s/#ifndef __MATLAB__/#if \!defined(__MATLAB__) \&\& \!defined(__ACADOIF__)/g" ./code_export_nmpc/acado.h