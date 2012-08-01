cat data1.dat | grep -v nan > dataClean1.dat
sed -i "1,3d" dataClean1.dat
#svn commit -m "new data"
