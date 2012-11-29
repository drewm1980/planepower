#!/usr/bin/env bash

(cd acado_public; git log -1 | grep git-svn-id | cut -d ' ' -f 6)
(cd acado_private; git log -1 | grep git-svn-id | cut -d ' ' -f 6)

echo "Bootstrapping ACADO..."
(
cd acado_public
cmake .
make -j3
#DO NOT INSTALL ACADO!!! IT INSTALLS SOME THINGS IN /usr/lib!!!
)

source acado_public/acado_env.sh
(
cd acado_private/testing/mvukov/mhe_export
cmake .
make -j3
)

# To build component, will need 
# FindACADO.cmake -- in main <ACADO_main>/cmake and
# FindMHEExport -- in mhe_export folder
# AND environment variables from:
#source extern/acado_public/acado_env.sh
#source extern/acado_private/testing/mvukov/mhe_export/mhe_export_env.sh
