#!/bin/bash

echo "This script will set up a component test directory structure in $1"

if [ ! -z $1 ]; then
	mkdir -p $1/comptest/bin
	mkdir -p $1/comptest/com.pelagicore.comptest/bin
	mkdir -p $1/comptest/com.pelagicore.comptest/shared
	mkdir -p $1/comptest/com.pelagicore.comptest/home
	mkdir -p $1/comptest/late_mounts

	sudo mount --bind $1/comptest/late_mounts $1/comptest/late_mounts
	sudo mount --make-unbindable $1/comptest/late_mounts
	sudo mount --make-shared $1/comptest/late_mounts
fi

