#!/usr/bin/env bash

# This script does most of the work to convert a component in the standard
# ROS/orocos package layout, (and thus depends on ROS's unstable and
# inefficient build infrastructure) to a simpler layout that builds with the
# tup build system.
#
# Oh, and it also assumes you're using git like most sane people these days ;)

x=`basename $1`

(
echo ""
echo "*** Upgrading component $x... ***"
cd $1

echo "Blowing away soon to be obsolete .gitignore file..."
touch .gitignore
find . -name ".gitignore" | xargs git rm --ignore-unmatch -q
rm -f .gitignore

echo "Replace the CMakeLists file with a simple Tupfile..."
find . -name CMakeLists.txt | xargs -n1 --no-run-if-empty git rm -q 
# This is a "herefile" that spits out a Tupfile.
# This may or may not be cleaner than copying a template Tupfile.
if [ -f Tupfile ]; then
	echo "Ignoring a pre-existing Tupfile..."
else
	echo "Generating a simple Tupfile..."
	(
	cat <<'EOF'
include_rules
: foreach *.cpp |> !cxx_orocos |>
: *.o |> !ld_orocos |> %d.so
EOF
	)>Tupfile
	git add Tupfile
fi

echo "Removing any obsolete Makefile..."
if [ -f Makefile ]; then
	echo "Found a makefile to blow away..."
	git rm -q --cached --ignore-unmatch -- Makefile
	rm -f Makefile
fi

echo "unROSify manifest.xml ..."
xmlstarlet sel -t -v //description manifest.xml >readme.txt
# Clean up some white space
sed -i -e :a -e '/./,$!d;/^\n*$/{$d;N;};/\n$/ba' readme.txt
git add readme.txt
find . -name "manifest*.xml" | xargs -n1 --no-run-if-empty git rm -q

echo "unROSify the script that test runs the component..."
# Note: the shebang will only work for you when/if some
# patches from Andrew Wagner make it into mainline.
echo "#!/usr/bin/env deployer" > test.ops
echo "" >> test.ops
opsname=`find . -name "*$x.ops" | head -n1`
cat $opsname >> test.ops
chmod +x test.ops
git add test.ops
git rm -q $opsname
echo "Blow away superfluous shell scripts..."
shname=`find . -name "*$x.sh" | head -n1 | grep -v cleanData.sh`
echo "Removing $shname..."
git rm -q --ignore-unmatch -- $shname rsinrsetne
echo "#!/usr/bin/env bash" > test.sh
echo "deployer-gnulinux -lerror -s test.ops" >> test.sh
chmod +x test.sh
git add test.sh

echo "Flattening the directory structure a bit..."
git mv -k src/* .
rmdir --ignore-fail-on-non-empty src
)

