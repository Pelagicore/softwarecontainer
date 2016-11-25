
# Examples

The examples are built when SoftwareContainer is configured with the option
`ENABLE_EXAMPLES=ON`.

This directory contains the following examples of how to work with and use
SoftwareContainer:

* simple - a script that launches a simple app inside a container.
* wayland - a script that launches weston and then some wayland test apps from inside a container
* temperature - a service and a client for a temperature service, with which the client can
  communicate from inside the container using the dbus gateway

## simple

After building the project, run the example from the directory
examples/simple e.g. like so:

    sudo ./launch.sh -b system
