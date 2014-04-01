#!/bin/bash

echo "This script will set up a component test directory structure in $1"

if [ ! -z $1 ]; then
	mkdir -p $1/bin
	mkdir -p $1/com.pelagicore.comptest/bin
	mkdir -p $1/com.pelagicore.comptest/shared
	mkdir -p $1/com.pelagicore.comptest/home
	mkdir -p $1/late_mounts

	sudo mount --bind $1/late_mounts $1/late_mounts
	sudo mount --make-unbindable $1/late_mounts
	sudo mount --make-shared $1/late_mounts
fi

