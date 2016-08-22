#!/bin/bash

pushd simple/
cmake . && make
./launch.sh -b system
popd
