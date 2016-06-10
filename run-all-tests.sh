#!/bin/bash

DIRECTORIES=(examples/ service-test/ build/)

for DIR in ${DIRECTORIES[@]}; do
	echo "Running tests in $DIR"
	pushd $DIR > /dev/null
	./run-tests.sh
	popd > /dev/null
done

