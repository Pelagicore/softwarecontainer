
.. _debugging:

Debugging
*********

The use of containers can make debugging a bit harder as we sometimes need to
get information from different sources of information. Paths can require some
help to parse for the computer as well as yourself

Debugging applications running in containers
============================================

This section and subsections describes some common ways of debugging
applications running inside containers and how to use common tools on the
applications running inside the containers.

LXC
---

LXC provides several tools that can be used to check out the containers
currently running on the system. `Note that all commands must be run as the main
user which the containers are started as, ie root usually.`

*lxc-ls* lists all currently running containers on the host.

*lxc-attach* can be used to "get inside" the container with a shell for example,
or to test run commands on the inside of the container. Please note that the
command given must be available inside the container.

*lxc-console* can be used to login with a shell inside the container in question.

*lxc-info* provides information about the running container, such as network
interface used for t he bridge, the state of the container, what PID it has and
so forth.

Cleanup
-------

The internal filesystem will always be deleted and recreated when a container
is restarted (destroyed and recreated). This means that anything you want to
save during debugging needs to be saved to a bind mounted directory before you
shut down the container.

``Note:`` This does not mean that the bind mounted filesystems will be deleted,
and large parts of the filesystems are usually bind mounted.

`SoftwareContainerAgent` will also delete all container instances upon startup,
meaning that container instances will be lost when restarting.

GDB
---

Attaching a `gdb` session can be done in two ways, one is to run the `gdb` session
and attach inside the container, the other is to attach to the process from the
outside of the container. Using both methods have their own set of problems you
need to solve.

Running GDB inside container
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you run `gdb` from inside the container using `lxc-attach`, you should not
need to change any paths etc to find the correct libraries etc. You will need
to have the `PID` from inside the container available to connect to however. this
can be found out by running::

    lxc-attach --name SC-X ps aux

Then running::

    lxc-attach --name SC-X -- gdb --pid YY

Replace YY with the `PID` as seen inside the container. You can also run the
application directly inside the container and start it with gdb directly::

    lxc-attach --name SC-X -- gdb --args /bin/ls

TODO: Uncertain why, but this fails with unable to find executable.

Running GDB outside container
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you run `gdb` from outside the container you will run into some problems with the
paths as the application is executing inside a `chroot` and `gdb` is executing
outside of it, this is fairly easily fixed however. First off you need to
provide `gdb` with the path of the executable as the path provided by the system
is inside the chroot::

    # ps aux |grep main
    # sudo gdb --pid XXX /tmp/container//SC-0/gateways/app/main

Begin with finding the global namespace `PID` of the application you are
debugging and then attach `gdb` to that `PID`, and the path to the application
in the `un-chrooted` filesystem.

The application should now be possible to debug as normal. Most of the standard
libraries etc are mounted inside the `chroot` from the outside and should be
available as normal without any changes. If you have dynamic libraries inside
your container that the application are linking to, you may need to add the
proper `solib-search-paths` to the libraries.

Running GDBserver on application running in container
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are running `SoftwareContainerAgent` on a target hardware you can still
debug applications inside the container using `gdbserver` which exports a gdb
socket on network and then you connect a gdb to that network socket and you can
debug using. Note that the `gdb` needs to be for the same architecture,
preferably also built together with the toolchain used to build for the target.

To perform this:

1. Start a container.
2. Configure network for the container.
3. Launch the app inside the container using ``gdbserver :7777 <appname>``.
4. The app will now wait for a gdb client to connect and start the application.
5. Start the appropriate gdb version with the binary name to debug
6. Type ``target remote <ip-of-container>:7777``
7. You are now connected to the gdbserver and can run the set breakpoints,
   start the application, etc.


Debugging using QtCreator inside a container
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Debugging applications running inside a container can be done, however, all
processes are run as root so you need to start `QtCreator` as root to be
able to connect to the process inside the container.

1. Start a container
2. Start `QtCreator` as root (sudo)
3. Open a project in `QtCreator`
4. Build the code into a runnable binary
5. Run the binary program inside the container
6. If you are using `DBUS`, a `PID` is returned, otherwise find it in the outside
   system.
7. Open *Debug -> Start Debugging -> Attach to Running Application...*
8. Find the right `PID` in the list, and attach.
9. The process will be halted and you can debug it from `QtCreator`.


Core dumps
----------
`SoftwareContainer` is configured to make the commands or functions that run inside the container
create a core dump in case of, for example, a segmentation fault.

The naming of a core dump file is the same inside a container as on the host, meaning this can
be set by looking at `/proc/sys/kernel/core_pattern` or by running `sysctl kernel.core_pattern`.
Because the container and host share a kernel, the setting on the host will be visible in the
container.

Note that the core_pattern can contain a path to where to store core dumps, so a wise decision from
a systems perspective, would be to set it to some directory which is then mounted into each
container, either by issuing `BindMount` calls on D-Bus, or by adding it as a FileGateway
configuration in a default capability (which would then be automatically enabled for every
container).

An alternative is to not use a pattern containing a path, and make sure you always execute your
application from a cwd which is also bind-mounted (which the application directory always is), and
then your core dumps will be stored together with the application binary.

More info about core dump patterns can be found in :linuxman:`core(5)`.

Nothing special should be needed to debug a core file using for example
`QtCreator` or `GDB`. Load the file as normal.

Debugging SoftwareContainerAgent
================================

Debugging the `SoftwareContainerAgent` should not be necessary for developers of
applications running inside containers managed by the `SoftwareContainerAgent`.
This is mainly interesting for developers of the actual
`SoftwareContainerAgent` and `SoftwareContainer` system.

Debugging the `SoftwareContainerAgent` should be straight forward, with the
exception that it runs as root, and hence all debugging tools also need to be
run as root until such a time that `SoftwareContainerAgent` can be run as a
non-root user.

Logging
-------

All the logs are performed using `ivi-logging`, meaning that `ivi-logging`
facilities can be used to filter logs. By default the logging is set to `DEBUG`
as of this writing.

Logs can be written to `DLT` backend if configured and compiled properly.


LXC
---

The `LXC` logs are controlled separately using two configuration options that
needs to be added to the `LXC` configuration, available in
*$CMAKE_INSTALL_PREFIX/etc/softwarecontainer.conf*::

    lxc.logfile
    lxc.loglevel

The `logfile` is a pointer to the file which the log will be stored in. The
valid `loglevels` are::

    FATAL
    ALERT
    CRIT
    ERROR
    WARN
    NOTICE
    INFO
    DEBUG
    TRACE

It can also be worthwhile checking the output from `lxc-checkconfig` to make sure
that your system is actually able to support all the `LXC` features that you
need.


GDB
---

`GDB` debugging of `SoftwareContainerAgent` should work with no issues. You do need
to run `GDB` as root however, as already mentioned.

Valgrind
--------

`Valgrind` of `SoftwareContainerAgent` works as expected.

Core dumps
----------

Coredumps can be debugged normally, no special usage necessary.

