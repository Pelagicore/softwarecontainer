Working with shared memory in containers
========================================

There are two main types of shared memory in Linux systems: POSIX shm and SysV IPC. POSIX shm
generally assumes that a there is a tmpfs mounted on ``/dev/shm`` where one can handle the shared
memory objects, which are accessed through its own family of API calls. POSIX shm is not
namespace-aware.

SysV IPC uses mechanisms built-in to the kernel (internally it uses a tmpfs as well, but it is
hidden). Among other things, SysV IPC supports shared memory, and is namespace-aware.

How to use this inside containers?
----------------------------------

POSIX SHM
#########

Using POSIX SHM requires you to have ``/dev/shm`` inside the container. To accomplish this, one
simply has to mount it, which should be done through the :ref:`file-gateway`.

SysV IPC
########

There is currently no easy way to, with the LXC API, stay in the same IPC namespace when creating a
new container. It is equally hard to attach to an existing IPC namespace (for example, one to use to
share data between host and container) when the container is already set up.

It is however possible, when using the ``attach()`` API call on the LXC container, to specify not to
enter a new IPC namespace, so it is indeed possible to share memory between host and container when
running a specific command. This would only be local to that command or application, and not to the
whole container. The init process and any other processes running inside the container that are not
instructed to share their IPC namespace would still live in a separate IPC namespace. This method is
however not implemented in SoftwareContainer, so currently any calls to ``attach()`` from
SoftwareContainer will always enter new namespaces (including the IPC namespace), as this is the
default behavior in LXC.
