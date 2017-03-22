systemd.service
===============

As it is mentioned in the :ref:`Getting Started <getting-started>` section the user API is implemented by
the SoftwareContainerAgent as a D-bus service. The API is used to start, configure, and control
containers. Thus in order to use SoftwareContainer containers, the SoftwareContainerAgent should be
run first. To be able to start the SoftwareContainerAgent and watch its process/system logs it would
be wise decision to use it as a systemd service.

systemd is an init system used in Linux distributions to bootstrap the user space. As of 2015, a large
number of Linux distributions adopt systemd as their default init system. More information about this
can be found `in freedesktop systemd docs <https://www.freedesktop.org/wiki/Software/systemd/>`_.

To add the agent to init system, the integrator should prepare a configuration file whose name ends
in ``.service``. This file encodes information about a process controlled and supervised by systemd.
The ``.service`` files are generally kept in the ``/lib/systemd/system`` directory.

There is a special syntax to prepare service files. More information about syntax can be found in
`freedesktop service unit configuration docs <https://www.freedesktop.org/software/systemd/man/systemd.service.html>`_
with examples.
