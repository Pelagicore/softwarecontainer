
# Service-tests

The service-tests runs SoftwareContainer as a binary and uses the Agent
D-Bus API to interact with it. Helpers and setup code is used to manage
the lifecycle of the container and often uses a separate application
to run inside the container to support the tests.

The service-tests depend on `py.test`


## D-Bus tests

The tests in `dbus/` tests various aspects of SoftwareContainer and the D-Bus
gateway. Not so much focus is put on configuration because that is mostly
tested on the `dbus-proxy` component itself which is a separate project.
Rather, focus is on load and stress tests from inside the container.

### Configuration requirements

This suite requires SoftwareContainer to be configured with the D-Bus gateway
enabled

### Running the tests

In the `dbus` directory, run the tests like so:

    sudo py.test


## Timing and profiling tests

These tests profile various timing in SoftwareContainer. The suite does not
resemble the other service-tests much, but the setup and framework is reused
from the other tests to the suite is integrated to the other service-tests
anyway.

This suite needs a test application to be configured and built with CMake.

### Configuration requirements

This suite requires SoftwareContainer to be configured with the D-Bus gateway
enabled

### Running the tests

In the `timing-profiling` directory, run the tests like so:

    sudo py.test
