#!/usr/bin/env bash

# List of orocos components
c=`ls|grep -v *.cpp|grep -v *.h|grep -v .sh | grep -v Tupfile`

for x in $c
do
	(
	echo "Upgrading component $x..."
	cd $x

	# Switch build system
	touch .gitignore
	find . -name ".gitignore" | xargs git rm --ignore-unmatch -q
	rm -f .gitignore
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

	# unROSify the manifest
	xmlstarlet sel -t -v //description manifest.xml >readme.txt
	sed -i -e :a -e '/./,$!d;/^\n*$/{$d;N;};/\n$/ba' readme.txt
	git add readme.txt
	git rm manifest*.xml -q

	# unROSify the scripts that "test" the component
	echo "#!/usr/bin/env deployer" > test.ops
	echo "" >> test.ops
	cat $x.ops >> test.ops
	chmod +x test.ops
	git add test.ops
	git rm $x.ops
	git rm run*.sh -q

	#echo "#!/usr/bin/env bash" > test.sh
	#echo "deployer-gnulinux -lerror -s test.ops" >> test.sh
	#chmod +x test.sh
	#git add test.sh

	# Flatten the directory structure a bit
	git mv src/* .

	)
done
