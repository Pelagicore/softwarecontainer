Write buffered filesystems
==========================

To enable the write buffers for the filesystems of an app, you need to specify
a short json object that is sent to the ``com.pelagicore.SoftwareContainerAgent.Create(id)``
method. What will happen is described in detail in the :ref:`Filesystem <filesystems>`
chapter.

Example
-------
The following snippet will create a container where a write buffer will be
created for all filesystems in that container::

    [{
        "writeBufferEnabled": true
    }]

The following example will disable write buffers for all filesystems within 
the container::

    [{
        "writeBufferEnabled": false
    }]

