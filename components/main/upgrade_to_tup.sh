#!/usr/bin/env bash
c=`ls|grep -v *.cpp|grep -v *.h|grep -v .sh | grep -v Tupfile |grep -v pythonCodegen`

echo "Will try to tupify the following components:"
echo $c

for x in c
do
	(
	echo "Upgrading component $x..."
	cd $x

	# Switch build system
	find . -name CMakeLists.txt | xargs git rm
	# This is a "herefile" that spits out a Tupfile.
	# This may or may not be cleaner than copying a template Tupfile.
	(
	cat <<'EOF'
include_rules
: foreach *.cpp |> !cxx |>
: *.o |> !ld |> %d.so
EOF
	)>Tupfile
	git add Tupfile

	# unROSify the manifest
	xmlstarlet sel -t -v //description manifest.xml >readme.txt
	sed -e :a -e '/./,$!d;/^\n*$/{$d;N;};/\n$/ba' readme.txt
	git add readme.txt
	git rm manifest*.xml

	# unROSify the scripts that "test" the component
	git mv $x.ops test.ops
	git rm run*.sh
	echo "#!/usr/bin/env bash">test.sh
	echo "deployer-gnulinux -lerror -s test.ops" >> test.sh
	chmod +x test.sh
	git add test.sh

	# Flatten the directory structure a bit
	git mv src/* .

	)
done
