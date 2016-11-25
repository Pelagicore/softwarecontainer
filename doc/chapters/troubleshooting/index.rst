
.. _troubleshooting:

Troubleshooting
***************

General
-------
* Make sure that the file ``/etc/mtab`` is a link to ``/proc/self/mounts``, or you may end up with
  strange mount issues.

FAQ
---

* When SoftwareContainerAgent starts it claims that it can not register the
  dbus interface.
    * In order for the SoftwareContainerAgent to work using the dbus system bus a
      policy file has to be installed in ``/etc/dbus/system.d/``. See the ``README.md``
      file for information on compile options.
    * You can also run the agent with ``--session-bus`` on the command-line, to run on the
      session bus instead.

* Why isn't pulseaudio working inside the container?
    * In order for pulseaudio to work inside the container it needs to be running when the container
      is started. This is the case because when softwarecontainer sets up the container it will look
      for the socket pointed out by ``PULSE_SERVER``, and mount it into the container, so that
      socket has to exist.
