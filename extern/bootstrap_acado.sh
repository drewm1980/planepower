#!/usr/bin/env bash

echo "Bootstrapping ACADO..."
(
cd acado_public
cmake .
make -j3
#DO NOT INSTALL ACADO!!! IT INSTALLS SOME THINGS IN /usr/lib!!!
)
(
cd acado_private
source ../acado_public/acado_env.sh
(cd testing/mvukov/mhe_export
cmake .
make -j3
)

# To build component, will need 
# FindACADO.cmake -- in main <ACADO_main>/cmake and
# FindMHEExport -- in mhe_export folder
# AND environment variables from:
#source extern/acado_public/acado_env.sh
#source extern/acado_private/testing/mvukov/mhe_export/mhe_export_env.sh
