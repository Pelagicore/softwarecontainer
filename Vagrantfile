# -*- mode: ruby -*-
# vi: set ft=ruby :
#

# Copyright (C) 2016-2017 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE
#

# Use absolute path so that we can run this from sub-directories
thisDir = File.dirname(__FILE__)
require File.join(thisDir, 'cookbook/host-system-config/vagrant-reload/plugin.rb')

Vagrant.configure(2) do |config|
    config.vm.box = "ubuntu/xenial64"

    # Common names for network adapters
    config.vm.network "public_network", bridge: [ "eth0", "eth1", "em1" ]

    config.vm.provider "virtualbox" do |vb|
        vb.cpus = 1
        vb.customize [ "guestproperty", "set", :id,
                       "/VirtualBox/GuestAdd/VBoxService/--timesync-set-threshold", 200 ]
        if ENV['VAGRANT_RAM'] then
            vb.memory = ENV['VAGRANT_RAM'].to_i * 1024
        end
    end

    # Sync the repo root with this path in the VM
    config.vm.synced_folder "./", "/home/ubuntu/softwarecontainer/", create: true

    # Workaround for some bad network stacks
    config.vm.provision "shell", privileged: false, path: "cookbook/utils/keepalive.sh"

    # Use apt-cacher on our apt cache server
    if ENV['APT_CACHE_SERVER'] then
        config.vm.provision "shell",
            args: [ENV['APT_CACHE_SERVER']],
            path: "cookbook/system-config/apt-cacher.sh"
    end

    # Make sure we have a fresh list from the package server
    config.vm.provision "shell", inline: "apt-get update"

    # Install build dependencies
    config.vm.provision "shell", path: "cookbook/system-config/setup-ccache-and-icecc.sh"
    config.vm.provision "shell", path: "cookbook/deps/common-build-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/softwarecontainer-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/common-run-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/wayland-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/pulseaudio-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/sphinx-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/pytest-and-dbus-testing-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/developer-tools.sh"

    # Change grub to support cgroups memory if necessary
    config.vm.provision "shell", path: "cookbook/system-config/grub-enable-cgroup-memory.sh"
    fileCheck = lambda {
        fileName = File.join(thisDir, "VAGRANT_NEEDS_RELOAD");
        if File.exists?(fileName)
            File.delete(fileName)
            return true
        else
            return false
        end
    }
    config.vm.provision "reload", condition: fileCheck

    # Add known hosts
    config.vm.provision "shell", privileged: false,
        path: "cookbook/system-config/ssh-keyscan-conf.sh"

    # Install dependencies via git
    config.vm.provision "shell", privileged: false,
        args: ["dlt-daemon", "https://github.com/GENIVI/dlt-daemon.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false,
        #args: ["ivi-logging", "https://github.com/Pelagicore/ivi-logging.git", "-DENABLE_DLT_BACKEND=1"],
        args: ["ivi-logging", "https://github.com/Pelagicore/ivi-logging.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false,
        args: ["dbus-proxy", "https://github.com/Pelagicore/dbus-proxy.git"],
        path: "cookbook/build/cmake-git-builder.sh"

end
