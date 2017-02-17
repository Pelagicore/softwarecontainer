Traffic Shaping
===============

Linux offers a very rich set of tools for managing and manipulating the transmission of packets.
Those tools can be used to manage and shape traffic on SoftwareContainer.

The Linux network routing, firewalling and traffic control subsystem is very powerful and flexible
and have grown to be a very mature stack of tools. To glance those solutions the linux
documentation project would be very good resource to start. Please check
`TLDP <http://www.tldp.org/HOWTO/html_single/Traffic-Control-HOWTO/#intro>`_
for documentation on the concepts of traffic control in Linux. `Linux Advanced Routing & Traffic Control <http://lartc.org/howto/>`_
is another good source that presents deep information about the topic.

Even though there are many ways of shaping the traffic, what we describe here is to give brief information
with examples about one particular way of shaping network traffic of a container to prevent possible
network starvation scenarios.

This can be done via the network classifier cgroup. The network classifier cgroup provides an interface
to tag network packets with a class identifier. It is possible to make each SoftwareContainer produce
network packages with different class id using different capabilities to set network classifier cgroup.
This way ``tc`` can be used to assign different priorities to packets from different cgroups.

The network classifier cgroup is supported to mark network packages by the :ref:`CGroups gateway <cgroups-gateway>`.

Example
-------
Here is an example way of marking network packages belonging to a container. The example aims to create
a SoftwareContainer, configure its related gateways to mark packages and finally present a small howto
about shaping traffic on host.

Create a service manifest to let container send/receive files on network and mark network classifier
cgroup. Then add this manifest to default manifest folder :

.. literalinclude:: examples/starvation_example.json


Then, from the build directory, run the agent:

.. literalinclude:: ../getting-started/examples/01_start_sc.sh


Next, we will start a new container:

.. literalinclude:: ../getting-started/examples/03_create_container.sh


Now we can set our prepared capabilities to mark network packages :

.. literalinclude:: examples/setcapabilities.sh


In this stage every network package within the container SC-0 will be marked with class id 0x100001. This
means every network package from this container will be marked with major class ID value 10 and minor
class ID value 1. So it is wise to configure traffic shaping with ``tc`` (please check `tc components <http://www.tldp.org/HOWTO/html_single/Traffic-Control-HOWTO/#components>`_
for more information about tc) according to indicated class IDs in the host :

.. literalinclude:: examples/tc.sh


With this setup SC-0 can consume only up to 1 mbit bitrate. For more information and examples it is
suggested to visit following

* http://lartc.org/howto/lartc.qdisc.filters.html
* http://www.tldp.org/HOWTO/html_single/Traffic-Control-HOWTO/
* https://www.kernel.org/doc/Documentation/cgroup-v1/net_cls.txt

