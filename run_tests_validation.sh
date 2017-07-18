#! /bin/bash

source ./local_software/env_vars.sh

cd run_tests_validation


# only execute scripts which do not start with an underscore
for i in [^_]*.sh [^_]*.py; do
	echo "******************************************************"
	echo "* Executing script $i"
	echo "******************************************************"
	./$i || exit
done

echo "******************************************************"
echo "******************************************************"
echo "******************************************************"
echo "*********************** FIN **************************"
echo "******************************************************"
echo "******************************************************"
echo "******************************************************"
