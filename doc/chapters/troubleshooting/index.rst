
.. _troubleshooting:

Troubleshooting
***************

Known issues
------------

* Currently, when configuring the project (with CMake) the option ``CMAKE_INSTALL_PREFIX``
  must be defined and set to '/usr', e.g: ``-DCMAKE_INSTALL_PREFIX=/usr``. If this is not
  done, there are issues starting the Agent properly.

