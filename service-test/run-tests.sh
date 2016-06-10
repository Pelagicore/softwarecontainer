#!/bin/bash

for dir in *; do
    if [ -d "$dir" ]; then
        if [ -e "${dir}/run-test.sh" ] ; then
            pushd $dir
            ./run-test.sh
            popd
        fi
    fi
done
