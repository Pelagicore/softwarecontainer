Network gateway
===============

The Network Gateway is used to setup network connection and configure which traffic is allowed and
not.

ID
--
The ID used for the Network gateway is: ``network``

Configuration
-------------
The configuration is structured as a list of JSON objects that each describe rules for ``OUTGOING``
or ``INCOMING`` network traffic. The validation is done by filtering network traffic on ``host``,
``ports`` and ``protocols``, where ``host`` is either hostname or ip address of a destination or
source depending on context, ``ports`` specifies which ports to filter on and ``protocols``
specifies which protocols to filter.

``direction`` and ``allow`` list are mandatory for Network Gateway configuration. There are only two
valid values for ``direction``: ``INCOMING`` and ``OUTGOING``. In each entry in ``allow`` list only
``host`` is mandatory.

``host`` can be specific hostname or ip address and also ``*`` indicating all available ip sources.

When ``ports`` is not specified, the rule applies to all ports.  ``ports`` are valid between 0 and
65536. Ports can be specified as a single port, as a string with a range ``"X-Y"``, or as a list
``[X,Y,Z]``.

There are three valid values for ``protocols``: ``"tcp"``, ``"udp"`` and ``"icmp"``. When
``protocols`` is not specified in the ``allow`` list item, the rule applies to all protocols. When
there is an item which has ports without protocols, the rule will be applied to all protocols.

The NetworkGateway is programmed to drop all packages that do not match any entry in ``allow`` list.

Sample use-cases
----------------

App has unlimited network access:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
::

  [
    {
      "direction": "OUTGOING",
      "allow": [
                 { "host": "*" }
               ]
    },
    {
      "direction": "INCOMING",
      "allow": [
                 { "host": "*" }
               ]
    }
  ]

App has limited network access
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Below is a simple browser example, allowing tcp access on ports 80 and 443 for any host.  Note that
for domain name resolution to work, traffic on port 53 is opened also.

::

  [
    {
      "direction": "OUTGOING",
      "allow": [
                 { "host": "*", "protocols": "tcp", "ports": [80, 443] },
                 { "host": "*", "protocols": ["udp", "tcp"], "ports": 53 }
               ]
    },
    {
      "direction": "INCOMING",
      "allow": [
                 { "host": "*", "protocols": ["udp", "tcp"], "ports": 53 }
               ]
    }
  ]

Below is an example where the app is able to use ICMP (so it would be able to ping), but nothing
else, also meaning no DNS!)

::

  [
    {
      "direction": "OUTGOING",
      "allow": [
                 { "host": "*", "protocols": "icmp" }
               ]
    },
    {
      "direction": "INCOMING",
      "allow": [
                 { "host": "*", "protocols": "icmp" }
               ]
    }
  ]

Below is a localhost example, where any traffic is allowed locally

::

  [
    {
      "direction": "OUTGOING",
      "allow": [
                 { "host": "127.0.0.1" }
               ]
    },
    {
      "direction": "INCOMING",
      "allow": [
                 { "host": "127.0.0.1" }
               ]
    }
  ]

