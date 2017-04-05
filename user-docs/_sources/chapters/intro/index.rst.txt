Welcome to the SoftwareContainer documentation
**********************************************

This is a developer's manual for SoftwareContainer. This documentation
describes how to use SoftwareContainer, both as a developer of the actual
software, but also as a developer of software that runs inside a container,
and as someone who wants to integrate towards SoftwareContainer with, for
example, an application launcher.

Formatting conventions for this document
========================================

* Short code examples, names of packages, etc are written like this: ``core-image-bistro``.
* Linux manual pages are referenced like this: :linuxman:`ls(1)`


References to the filesystem
============================

Throughout the docs, there are some references to concrete locations of files and other
paths in the file system. These are set by the build system and should only be considered
correct for the specific build that built the docs. This means that in the case of reading
these docs e.g. online, the paths are only for reference and not necessarily how it will
look if built locally or for target.
