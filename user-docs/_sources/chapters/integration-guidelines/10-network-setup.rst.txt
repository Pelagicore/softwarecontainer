Network setup
=============

The network setup of SoftwareContainer is dependent on a network bridge being available on the
host system, if compiled with support for the NetworkGateway. By default, SoftwareContainer will
create such a bridge on the system if it is not already there. This can be changed, so that
SoftwareContainer will simply fail with an error message if the bridge was not available.

The selection of whether or not to create the bridge is a compile-time option given to CMake.
Please see the README for more information about how to set the various CMake options.

For each container a virtual ethernet device will be set up and be bridged to the above mentioned
network bridge on the host system. The virtual ethernet device is then mapped to an ethernet device
inside of the container, configured to be eth0. The LXC template also copies ``/etc/resolv.conf``
from the host into the container, if it is available in the host. That means the same name servers
will be used in the container as on the host.

In order to configure what traffic is allowed the NetworkGateway is used. The NetworkGateway
converts the configuration it receives into iptables rules which are set for the network device
inside of the container. See :ref:`Gateways <gateways>` for more information.

Network setup sequence
----------------------
On a typical IVI-system, this is how network setup will most likely look, from a SoftwareContainer
perspective.

#. System boots

   #. System sets up whatever network settings are needed in general

#. SoftwareContainer starts

    #. SoftwareContainer reads all available service manifests

#. SoftwareContainer sets up a bridge interface (if configured to do so)

   #. Bridge interface name and ip settings are read from the main configuration file
   #. SoftwareContainer sets up NAT between the host and the network defined by the bridge

.. actdiag::

    boot -> systemNetworkSetup ->
    startSC ->
        readManifests ->
    SCCreateBridge ->
    SCConfigureNAT;

    lane System {
        boot [label = "System boot"];
        systemNetworkSetup [label = "System network setup"];
        startSC [label = "Start\n SoftwareContainer"];
    }

    lane SoftwareContainer {
        readManifests [label = "Read Manifests"];
        SCCreateBridge [label = "Create Bridge"];
        SCConfigureNAT [label = "Configure NAT"];
    }

All of the above will be done globally from a SoftwareContainer perspective, meaning it will not be
done per-container. The below steps are per-container

#. User starts an app somehow (probably through a launcher)
#. Launcher creates a container

   #. LXC creates a veth interface on the host and connects it to the bridge
   #. LXC creates a network interface inside the container, and pairs it with the host interface.

#. Launcher sets app capabilities

    #. All gateway configs that are default are read
    #. All gateway configs that are given are read
    #. The gateway configs are applied
    #. The network interface for the container is brought up and given ip address
       Note: the IP is automatic, based on container ID
    #. The network gateway applies all the iptables rules given in the configs given by the
       capabilities - inside the container.

#. Launcher starts app in container
#. App runs
#. App stops (probably called by the launcher)
#. Container is destroyed

    #. LXC brings down and deletes the container network interface
    #. LXC deletes the veth interface on the host


.. actdiag::

    launchApp -> createContainer -> LXCVethHost -> LXCVethContainer ->
    setCapabilities -> defaultRead -> capsRead -> applyCaps -> networkIfaceUp ->
    networkGWRules -> appStarts -> appRuns -> appStops ->
    LXCDeleteVethCont -> LXCDeleteVethHost -> containerDestroyed;

    lane Launcher {
        launchApp [ label = "App launched" ];
        setCapabilities [ label = "Call SetCapabilities" ];
        appStarts [ label = "Run app binary" ];
        appStops [ label = "Stop app" ];
    }

    lane SoftwareContainer {
        createContainer [ label = "Container created" ];
        defaultRead [ label = "Reads default caps" ];
        capsRead [ label = "Reads given caps" ];
        applyCaps [ label = "Caps applied" ];
        containerDestroyed [ label = "Container destroyed" ];
    }

    lane LXC {
        LXCVethHost [ label = "Veth created\n in host" ];
        LXCVethContainer [ label = "Veth created\n in container" ];
        LXCDeleteVethCont [ label = "Delete veth\n in container" ];
        LXCDeleteVethHost [ label = "Delete veth in host" ];
    }

    lane Container {
        networkIfaceUp [label = "Veth in \n container UP" ];
        networkGWRules [label = "Iptables applied" ];
        appRuns [ label = "App runs" ];
    }

