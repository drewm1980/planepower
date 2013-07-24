# This script checks out, builds, and reinstalls tup.
echo "Bootstrapping tup..."

sudo apt-get install fuse libfuse-dev

echo "Note, you need to add user to fuse group for tup to work properly"

buildloc=~/src/
mkdir -p $buildloc
(
cd $buildloc
if [ ! -d $buildloc/tup ]; then
	git clone git://github.com/gittup/tup.git
fi
(
cd tup
git pull
./bootstrap.sh
sudo cp build/tup /usr/local/bin/
sudo cp tup.1 /usr/local/share/man/man1/
)
)
echo "Done bootstrapping tup"

