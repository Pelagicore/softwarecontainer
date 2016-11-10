
.. _troubleshooting:

Troubleshooting
***************

FAQ
---

* When SoftwareContainerAgent starts it claims that it can not register the
  dbus interface.
** In order for the SoftwareContainerAgent to work using the dbus system bus a
   policy file has to be installed in "/etc/dbus/system.d/". See the README
   file for information on compile options.
