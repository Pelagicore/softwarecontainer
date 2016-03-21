# -*- mode: ruby -*-
# vi: set ft=ruby :

vagrant_private_key_file="vagrant_key"

Vagrant.configure(2) do |config|
    config.vm.box = "debian/jessie64"

    # Deploy a private key used to clone gits from pelagicore.net
    config.vm.provision "file", source: vagrant_private_key_file, destination: "/home/vagrant/.ssh/id_rsa"

    # Install dependencies via apt
    config.vm.provision "shell", inline: <<-SHELL

        # Several packages need to be very recent, so use debian testing
        sed -i 's/jessie/testing/g' /etc/apt/sources.list
        apt-get update
        DEBIAN_FRONTEND=noninteractive apt-get -o Dpkg::Options::="--force-confnew" --force-yes -fuy dist-upgrade

        # Common dependencies
        apt-get install -y git cmake build-essential pkg-config

        # Common for ivi-logging & pelagicore-utils
        apt-get install -y libglib2.0-dev

        # For pelagicontain
        apt-get install -y libdbus-c++-dev libdbus-c++-1-0v5 libdbus-1-dev libglibmm-2.4-dev libglibmm-2.4 lxc-dev libpulse-dev unzip bridge-utils

        # For jsonparser
        apt-get install -y libjansson-dev libjansson4

        # For pelagicontain examples
        apt-get install -y dbus-x11
    SHELL

    # Add known hosts
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        ssh-keyscan git.pelagicore.net >> ~/.ssh/known_hosts
        ssh-keyscan github.com >> ~/.ssh/known_hosts
    SHELL

    # Install dependencies via git
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        # IVI Logging
        git clone https://github.com/Pelagicore/ivi-logging.git
        cd ivi-logging
        mkdir build
        cd build
        cmake ..
        make && sudo make install

        cd

        # Pelagicore-utils
        git clone git@git.pelagicore.net:development-tools/pelagicore-utils.git
        cd pelagicore-utils
        mkdir build
        cd build
        cmake ..
        make && sudo make install

        cd

        #ivi-mainloop
        git clone https://github.com/Pelagicore/ivi-main-loop.git
        cd ivi-main-loop
        mkdir build
        cd build
        cmake ..
        make && sudo make install

        # jsonparser
        git clone git@git.pelagicore.net:pelagicore-utils/jsonparser.git
        cd jsonparser
        mkdir build
        cd build
        cmake ..
        make && sudo make install
    SHELL

    # Install project
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        git clone git@git.pelagicore.net:application-management/pelagicontain.git
        cd pelagicontain
        mkdir build
        cd build
        cmake ..
        make && sudo make install
    SHELL

    # Run an example (note, running as root)
    config.vm.provision "shell", inline: <<-SHELL
        cd pelagicontain/examples/simple
        make
        ./launch.sh
    SHELL
end
