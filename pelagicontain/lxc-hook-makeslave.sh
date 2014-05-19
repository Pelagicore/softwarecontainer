#!/bin/sh

# MOUNT_DIR is set by Pelagicontain when running lxc-execute
mount --make-slave $MOUNT_DIR
