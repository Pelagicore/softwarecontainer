/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */
/*!
\mainpage

<h2>Introduction</h2>
Pelagicontain is a tool for launching contained Linux applications.
The API provided by Pelagicontain is an abstraction on top of the specific
containment implementation and provides an abstraction of the conceptual
phases 'preload', 'launch', and 'shutdown'. 

Applications run within a container are executed in a chrooted environment with
limited access to devices, files, and D-Bus. A good rule of thumb is to assume all
access is closed unless explicitly opened.

In contrast to full virtualization, applications run in a container all access
the same kernel and have access (when configured) to the <i>real</i> devices of
the system. Just as in a fully virtualized environment, applications executing
in different environments cannot access each other and pids from one container
are not visible from within another. Resources such as memory, CPU and network can be restricted and portioned on a
per-container basis.

Restrictions and access is controlled by gateways. Each gateway is started
together with the container during the preload phase. The full gateway functionality
is enabled during the launch pahse when each respective gateway gets its
configuration from Platform Access Manager. When gateways have been configured and
enabled the application is launched in a fully contained environment. 

The preload and launch phases are normally initiated by the launcher, e.g.
Application Manager. As part of preloading, Pelagicontain sets up a container
and executes the Controller inside it, and preloads the gateways by starting
them without any configuration, i.e up and running but completely closed.

During launch Pelagicontain registers as a client with Platform Access Manager
to get the relevant gateway configurations based on what capabilities the
application to be launched has. When all gateway configurations are set and the
gateways are enabled, Pelagicontain tells Controller to launch the application.

During the shutdown phase which also normally is initiated by the launcher,
Pelagicontain tells Controller to shut down the application, brings down all
gateways, and unregisters as a client with Platform Access Manager. Pelagicontain
shuts down itself after this is completed.

<h2>The structure of a container</h2>
Assuming the container location (i.e. the contanier root directory) is called
\c $containerRoot

\c $containerRoot/\<appId\> must contain the following directories:
<ul>
	<li> \c bin/ </li>
		<ul><li>
		This is where the binaries exposed to the container should
		be placed. This includes libraries required for execution.
		</li></ul>
	<li> \c shared/ </li>
		<ul><li>
		This is where shared data exposed to the container should
		be placed, files that should be shared between all instances
		of a running application.
		</li></ul>
	<li> \c home/ </li>
		<ul><li>
		This is where shared data exposed to the container should
		be placed, files that should be shared between all instances
		of a running application.
		</li></ul>		
</ul>

When launching an instance of pelagicontain a random containerId will be generated
 which is valid only for that session. 

In \c $containerRoot/$containerId/gateways/ the different gateways can create e.g.
sockets used to communicate in and out of the container. This directory is available
inside the container as /gateways/. These files and directory
and automatically cleaned up on exit. An examples of files in /gateways/ is:

<ul>
	<li>Session D-Bus proxy socket</li>
		<ul><li>
		This is the socket for communicating with the D-Bus session
		bus. Traffic is filtered by the DBusGateway
		</ul></li>
	<li>System D-Bus proxy socket</li>
		<ul><li>
		This socket works in the same way as the session socket. Each
		socket is connected to a separate D-Bus proxy process.
		</ul></li>

</ul>

<h2>Running Pelagicontain</h2>
The main program of Pelagicontain is called \c pelagicontain, and is used to
launch contained applications. \c pelagicontain is invoked with the base
directory of the launcher as the first parameter and the command to run
within the container as the second parameter, and a unique cookie string as
a third argument.

Before running \c pelagicontain a "runtime directory" needs to be created and
set up. The run-time directory should look like this: <br>

<code>
├── bin<br>
│   └── controller<br>
├── \<appId\><br>
│   ├── bin<br>
│   │   └── containedapp<br>
│   ├── home<br>
│   └── shared<br>
└── late_mounts<br>
</code>

The directory late_mounts needs to be set up in a specific way in order to 
allow propagation of mount points into a already started container. This 
is done in the following way (as root):

<code>mkdir -p \<runtime dir\>/late_mounts</code><br>
<code>mount --bind \<runtime dir\>/late_mounts \<runtime dir\>/late_mounts</code><br>
<code>mount --make-unbindable \<runtime dir\>/late_mounts</code><br>
<code>mount --make-shared \<runtime dir\>/late_mounts</code><br>

As listed in the overview of the runtime directory above, a \<appId\> directory
containing three sub-directories needs to be created. The bin directory should
contain the actual application that will be started and any libraries needed,
and will be available as /appbin/ inside the container. Shared and home will
also be available inside the container as /appshared/ and /apphome/.

A network bridge should be set up as well:
<code>brctl addbr container-br0</code>

Running:

\c pelagicontain \c $containerRoot \c myCommand \c cookie

will invoke the \c myCommand in a container which will be created with
\c $containerRoot as root directory. 

In the normal case where Pelagicontain is run as part of the platform, the
binary to run inside the container will be the \c controller which will later
start the contained application. The possibility to pass
another command is kept as it makes some component testing easier.

<h2>Running the Pelagicontain component tests</h2>
<code>mkdir -p /tmp/test/</code><br>
<code>cp -R pelagicontain/component-test/test-data/comptest/\* /tmp/test/</code>

Copy \c controller to <code>/tmp/test/bin/</code><br>
Copy \c containedapp to <code>/tmp/test/com.pelagicore.comptest/</code> (containedapp is
built separately from the pelagicontain project and is found in
pelagicontain/component-test/)<br>
Set up <code>/tmp/test/late_mounts</code> the same way described under "Running Pelagicontain".

Add a container-br0 bridge:
<code>brctl addbr container-br0</code>

With root privilegies start \c pam_stub.py (found in pelagicontain/component-test/)

Run \c test_pelagicontain (also with root privilegies) and point out where the
 \c pelagicontain binary is (assuming we are in the git repo root and build is
 done in \c build):<br>
<code>PC_BINARY=build/pelagicontain/src/pelagicontain ./pelagicontain/component-test/test_pelagicontain.py</code>

Remember that pam_stub and the test must have access to the same D-Bus bus,
and the test requires root privilegies.

<h2>Per-container Configuration using pelagicontain.conf</h2>
\deprecated This needs to be reviewed and updated, only partially relevant

pelagicontain.conf is the main configuration file for a container. Settings
written here are only applied to the current container.

This file is written in JSON format and is parsed internally by the
pelagicontain program. Pelagicontain interprets the configuration parameters
and configures underlying systems such as e.g. LXC.

<h2>System-wide configuration using /etc/pelagicontain</h2>
\deprecated This needs to be reviewed and updated, only partially relevant

There are two system-wide configuration files for Pelagicontain,
\c /etc/pelagicontain and \c lxc-pelagicontain. These two files directly influence
the underlying LXC system. \c /etc/pelagicontain is an LXC configuration file
which path is passed to LXC on creation.

<h2>System-wide configuration using the lxc-pelagicontain template</h2>
\deprecated This needs to be reviewed and updated, only partially relevant

Prior to starting a container some initialization of the environment has to be
made. This includes creating the root file system to be exposed to the
container. A minimal root filesystem needs to contain the directories used for
mount points and also a subset of the device nodes of the host system.

Changes made to the guest file system, or to the global device node
configuration are best placed in \c $datarootdir/templates/lxc-pelagicontain.

The exact location of lxc-pelagicontain can vary depending on how LXC is
configured to be installed. On a Debian machine it is typically located in \c
/usr/share/lxc/templates/lxc-pelagicontain

*/
