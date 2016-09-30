#!/bin/bash
# A collection of useful scripts that deal with lxc

function shutdownContainers {
    # Ask all lxc containers to stop
    for L in `lxc-ls -1 --running`
    do
        lxc-stop --nowait -n "$L"
    done

    # Kill all lxc containers that did not stop
    for L in `lxc-ls -1 --running`
    do
        lxc-stop --kill -n "$L"
    done

    # Destroy all lxc containers
    for L in `lxc-ls -1`
    do
        lxc-destroy -n "$L"
    done
}

shutdownContainers
