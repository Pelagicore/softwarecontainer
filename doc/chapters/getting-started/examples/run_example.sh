#!/bin/sh -xe
# 
# Prerequisites:
#  * softwarecontainer is installed together with examples
#  * temperature service example is installed in
#    /usr/local/share/softwarecontainer/examples
#  * service manifests are stored in
#    /usr/local/etc/softwarecontainer/service-manifest.d

eval `dbus-launch --sh-syntax`

# Prepare environment. Copy the service manifest for the temperature service
# into the service manifest directory, this is deleted after the tests are run.
#
# This is done so we don't clutter the other files with unecessary command line
# options etc, we could use -d or -m instead, but it would also wind up in the
# actual chapter. 
#
sudo cp /usr/local/share/softwarecontainer/examples/temperature/temperature-service.json \
    /usr/local/etc/softwarecontainer/service-manifest.d 

./01_start_sc.sh &
sc_pid=$!
sleep 1
./02_introspect.sh
./03_create_container.sh
./04_bindmount.sh
./05_execute.sh
./06_suspend.sh
./07_resume.sh
./08_listcapabilities.sh
./09_setcapabilities.sh
./10_destroy.sh

sudo pkill -f "softwarecontainer-agent"

# Remove copied files
sudo rm /usr/local/etc/softwarecontainer/service-manifest.d/temperature-service.json

