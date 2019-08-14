#!/bin/sh
# Run all unit tests
# The name of each qt project file should be the same as that of its containing
# directory. For example, unit test "testversioncompare" has the following 
# directory hierachy:
#    testversioncompare/
#       testversioncompare.pro
#       testversioncompare.cpp
#       testversioncompare.h

TMP_OUTPUT=tmp-output.txt

run_command() {
	"$@"
	STATUS=$?
	if [ $STATUS -ne 0 ]; then
		cat $TMP_OUTPUT
		rm -f $TMP_OUTPUT
		echo "$1 returned exit status $STATUS"
		exit 1
	fi
	rm -f $TMP_OUTPUT
}

run_test() {
	DIR="$1"
	cd "$DIR"
	if [ -f "${DIR}.pro" ]; then
		echo "running test: ${DIR}"
		run_command qmake
		run_command make
		run_command ./${DIR}
	else
		echo "warning: ${DIR}/${DIR}.pro not found"
	fi
	cd ..
}

for dir in *; do
	if [ -d "$dir" ]; then # it is a directory
		run_test "$dir"
	fi
done
