#!/usr/bin/env bash

# This script does most of the work to convert a component in the standard
# ROS/orocos package layout, (and thus depends on ROS's unstable and
# inefficient build infrastructure) to a simpler layout that builds with the
# tup build system.
#
# Oh, and it also assumes you're using git like most sane people these days ;)

x=`basename $1`

(
echo "Upgrading component $x..."
cd $1

# Blow away soon to be obsolete .gitignore files
touch .gitignore
find . -name ".gitignore" | xargs git rm --ignore-unmatch -q
rm -f .gitignore

# Replace the CMakeLists file with a simple Tupfile
find . -name CMakeLists.txt | xargs git rm -q
# This is a "herefile" that spits out a Tupfile.
# This may or may not be cleaner than copying a template Tupfile.
(
cat <<'EOF'
include_rules
: foreach *.cpp |> !cxx_orocos |>
: *.o |> !ld_orocos |> %d.so
EOF
)>Tupfile
git add Tupfile

# unROSify manifest.xml
xmlstarlet sel -t -v //description manifest.xml >readme.txt
sed -i -e :a -e '/./,$!d;/^\n*$/{$d;N;};/\n$/ba' readme.txt
git add readme.txt
git rm manifest*.xml -q

# unROSify the scripts that "test" the component.
# Note: the shebang will only work for you when/if some
# patches from Andrew Wagner make it into mainline.
echo "#!/usr/bin/env deployer" > test.ops
echo "" >> test.ops
cat $x.ops >> test.ops
chmod +x test.ops
git add test.ops
git rm $x.ops
git rm run*.sh -q

# Output a test.sh file just in case the deployer
# does ~not have the above mentioned patch, or
# the user wants to modify the debug level.
echo "#!/usr/bin/env bash" > test.sh
echo "deployer-gnulinux -lerror -s test.ops" >> test.sh
chmod +x test.sh
git add test.sh

# Flatten the directory structure a bit
git mv src/* .
rmdir src
)
