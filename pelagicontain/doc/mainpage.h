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
limited access to devices and files. A good rule of thumb is to assume all
files and devices not specifically given access to are inaccessible.

In contrast to full virtualization, applications run in a container all access
the same kernel and have access (when configured) to the <i>real</i> devices of
the system. Just as in a fully virtualized environment, applications executing
in different environments cannot access each other and pids from one container
are not visible from within another.

Resources such as memory, CPU and network can be restricted and portioned on a
per-container basis.

Restrictions and access is controlled by gateways. Each gateway is started
together with the container during the preload phase. The full gateway functionality
is enabled during the launch pahse when each respective gateway gets its
configuration from Platform Access Manager. When gateways have been configured,
they are enabled and the contained application is launched in a fully contained
environment. 

The preload phase is normally initiated by the launcher, e.g. Application Manager.
As part of this phase, Pelagicontain sets up a container by launching the
Controller inside it, and preloads the gateways.

The launch pahse is also normally initiated by the launcher. During this phase
Pelagicontain registers as a client with Platform Access Manager to get the
relevant gateway configurations based on what capabilities the application
to be launched has. When all gateway configurations are set and the gateways
are enabled, Pelagicontain tells Controller to launch the application which is
the end of the launch phase.

During the shutdown phase which also normally is initiated by the launcher,
Pelagicontain tells Controller to shut down the application, brings down all
gateways, and unregisters as a client with Platform Access Manager. Pelagicontain
shuts down itself after this is completed.

<h2>Running Pelagicontain</h2>
The main program of Pelagicontain is called \c pelagicontain, and is used to
launch contained applications. \c pelagicontain is invoked with the base
directory of the container as the first parameter and the command to run
within the container as the second parameter, and a unique cookie string as
a third argument. Pelagicontain's functionality in general assumes the command
to run is to start Controller inside the container. 

\c pelagicontain \c containerRoot \c myCommand \c cookie

will invoke the \c myCommand in a container which will be created with
\c containerRoot as root directory. It is
important to note that all commands and libraries needed in the container must
be deployed to a directory named \c rootfs in the \c containerRoot directory,
i.e. a binary that should be executed inside the container should be placed
in \c containerRoot/rootfs/. This directory will be mounted as \c /deployed_app
inside the container.

In the normal case where Pelagicontain is run as part of the platform, the
binary to run inside the container will be the \c controller which will later
start the contained application.

<h2>The structure of a container</h2>
A \c $container_base_dir must contain the following directories:
<ul>
	<li> \c rootfs/ </li>
		<ul><li>
		This is where the data exposed to the container should
		be placed. This includes binaries and libraries required for
		execution, but also other resources such as graphics and
		audio.
		</li></ul>
</ul>

In \c rootfs/ the following files are created, and cleaned up on exit:
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

<h2>Per-container Configuration using pelagicontain.conf</h2>
pelagicontain.conf is the main configuration file for a container. Settings
written here are only applied to the current container.

This file is written in JSON format and is parsed internally by the
pelagicontain program. Pelagicontain interprets the configuration parameters
and configures underlying systems such as e.g. LXC.

<h3>Sample configuration</h3>
The following configuration illustrates the most important concepts of
pelagicontain.conf.

\verbatim
{
    "dbus-proxy-config-session": [
        {
            "direction": "*",
            "interface": "*",
            "object-path": "*",
            "method": "*"
        }
    ],
    "dbus-proxy-config-system": [
        {
           "direction": "*",
           "interface": "*",
           "object-path": "/org/bluez/*",
           "method": "*"
        },
        {
           "direction": "*",
           "interface": "org.bluez.Manager",
           "object-path": "/",
           "method": "*"
        }
    ],
    "iptables-rules": [
	"iptables -I FORWARD --src $SRC_IP  -j ACCEPT",
	"iptables -I FORWARD --dest $SRC_IP -j ACCEPT"
    ],
    "lxc-config-template": "/etc/pelagicontain",
    "bandwidth-limit":     "500kbps",
    "ip-addr-net":         "192.168.100.",
    "gw-ip-addr":          "192.168.100.1"
}
\endverbatim

<h4>D-Bus restrictions</h4>
\c dbus-proxy-config-session and the \c
dbus-proxy-config-system sections are used to specify the configuration of
\c dbus-proxy. These sections allows specifying which D-Bus services the
container should have access to. Everything not explicitly whitelisted is
disallowed.

The \c direction parameter specifies whether the rule concerns \c incoming or
\c outgoing traffic.

The rest of the parameters have the same meaning as in any other D-Bus
application.

<h3>IP restrictions</h3>
The \c iptables-rules section allows specifying IPTables rules which will be
executed upon setup of the container. The special variable $SRC_IP is provided
with the IP of the container.

<h3>General networking</h3>
\c bandwith-limit specifies the incoming and outgoing bandwidth separately.
Allowed suffixes here are the same as for the \c tc command, kbps and mbps are
believed to be sufficient for most users.

\c ip-addr-net specifies the 24-bit network part of the IP address of the
container. The remaining bits are randomized upon startup of the container.

\c gw-ip-addr specifies the defaul gateway for the container.

<h3>Other</h3>
\c lxc-config-template specifies the path to the system-wide LXC configuration
file. This file is used as a template for further configuration. Anything added
to this file will be applied to all containers.

<h2>System-wide configuration using /etc/pelagicontain</h2>
There are two system-wide configuration files for Pelagicontain, \c
/etc/pelagicontain and \c lxc-pelagicontain. These two files directly influence
the underlying LXC system. \c /etc/pelagicontain is an LXC configuration file
which is used as a template for all container-specific configurations. This
file is essentially pre-pended to all container configs.

This is a good place to store LXC configuration parameters global to all
containers.

<h2>System-wide configuration using the lxc-pelagicontain template</h2>
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
