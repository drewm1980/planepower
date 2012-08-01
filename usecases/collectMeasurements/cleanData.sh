cat data.dat | grep -v nan > dataClean.dat
sed -i "1,3d" dataClean.dat
#svn commit -m "new data"
