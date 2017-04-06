Resource Management in containers
=================================

Resource Management can be configured for containers by using the ``cpu.shares``
CGroup setting.
This results in that the CPU time available to all containers will be distributed according
to the values specified for each container.
