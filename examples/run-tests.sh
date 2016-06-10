#!/bin/bash

pushd simple/
cmake . && make
./launch.sh -b session
popd
