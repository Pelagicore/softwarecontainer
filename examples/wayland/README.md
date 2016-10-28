Example for running wayland applications in SoftwareContainer

Requirements
============
* weston
* weston-simple-egl and weston-flower examples
* softwarecontainer

How to run
==========
```
# ./launch.sh
```

Known issues
============
* Running the example from ssh in a VM does not seem to work, because
  weston refuses to start without a proper tty.
** WORKAROUND: Have X11 or another wayland compositor running first
   and run the example from xterm or weston-terminal (for example).

