This package includes the Pelagicontain component


After cloning run this to initialize the submodules:

git submodule update --init


To build the project documentation set -DENABLE_DOC=1 when running cmake.

The project Doxygen docs contain info about Pelagicontain in general,
how to run Pelagicontain and how to run the component tests.

To run the unit-tests build the project with -DENABLE_COVERAGE=1 -DBUILD_TESTS=ON


NOTE ON D-BUS RUNNING ON DESKTOP SYSTEMS

You need to do some D-Bus magic in order to get this running. The problem is:
Typically you need root access to start pelagicontain (since the lxc commands
need special privileges). You might then run pelagicontain using sudo, which is
fine, but when you do, your DBUS_SESSION_BUS_ADDRESS will not be set,
dbus-launch has not been executed for root yet in your sudo session. What
you'll need to do is to actually log in as root and start a dbus session prior
to launching pelagicontain. Alternatively you can wrap pelagicontain in a bash
script which runs dbus-launch.

Example:

sudo su
eval `dbus-launch --sh-syntax`
pelagicontain /home/myuser/deploy /deployed_app/dbus_app
exit

CONFIGURING THE DBUS PROXY
The D-Bus proxy is launched "outside" the container, and will look for two rule
files. One for the SESSION bus, and one for the SYSTEM bus. The configuration
files should be placed in the directory used for app deployment, so, if you
ran:
pelagicontain /tmp/vim_app /deployed_app/vim

.. then configs are placed in /tmp/vim_app. The configs are named:
sess_proxy_config  -- for SESSION rules
sys_proxy_config   -- for SYSTEM rules


NOTE ON PULSEAUDIO

In order to pulseaudio to work inside the container it needs to be running when
the container is started. This is the case because when pelagicontainer sets
up the container it will create a new socket to the pulseaudio server.
