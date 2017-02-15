#!/bin/sh -xe 

eval `dbus-launch --sh-syntax`

./01_start_sc.sh &
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

sudo pkill -s 0

