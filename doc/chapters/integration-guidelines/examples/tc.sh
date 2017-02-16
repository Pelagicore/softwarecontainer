sudo tc qdisc add dev eth0 root handle 10: htb
sudo tc class add dev eth0 parent 10: classid 10:1 htb rate 1mbit
sudo tc filter add dev eth0 parent 10: protocol ip prio 10 handle 1: cgroup
