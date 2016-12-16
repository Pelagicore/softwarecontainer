Environment gateway
===================

The Environment Gateway is used to set environment variables in the container.

The environment gateway allows users to specify environment variables that
should be known to the container and all commands and functions running
inside the container.

Note that any environment variables set here can be overridden when starting a binary on the
D-Bus interface, see :ref:`D-Bus API <dbus-api>`.

ID
--

The ID used for the Environment gateway is: ``env``

Configuration
-------------

The configuration consists of a list of environment variable definitions. Each
such element must contain the following parameters:

- ``name`` The name of the environment variable in question
- ``value`` The value to attach to the name

Note that ``value`` will be read as a string.

If a variable is added that has previously been added already, the new value is ignored
and the old value is kept intact. This is considered a misconfiguration and will generate
a log warning.

The configuration may also, optionally, specify the following parameters:

- ``mode``: A string that can be either:
    - ``set``: set the variable (this is the default, so one does not usually need to set it)
    - ``append``: append the value given to the previous value for the given variable.
    - ``prepend``: prepend the value given to the previous value for the given variable.
- ``separator``: A string that will be squeezed in between the old and new value if one wants to
                 prepend or append to a variable

Both ``append`` and ``prepend`` will have the same effect as ``set`` if the variable was not already
set.

Example configurations
----------------------

En example configuration would look like this::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "SOME_VALUE"
        }
    ]

With the above configuration, ``SOME_ENVIRONMENT_VARIABLE`` would be set to ``SOME_VALUE``,
if the variable had not been previously set. In the case where ``SOME_ENVIRONMENT_VARIABLE``
would have been previously set to e.g. ``ORIGINAL_VALUE``, that value would be kept.

There is also the possibility to append to an already defined variable::

    [
        {
            "name": "SOME_ENVIRONMENT_VARIABLE",
            "value": "/some/path",
            "mode": "append"
            "separator": ":"
        }
    ]

With the above configuration, if ``SOME_ENVIRONMENT_VARIABLE`` had previously been set
to e.g. ``/tmp/test``, the varaiable value would now be set to ``/tmp/test:/some/path``.
If ``SOME_ENVIRONMENT_VARIABLE`` had not been previously set, the value would now be
set to ``:/some/path``.


