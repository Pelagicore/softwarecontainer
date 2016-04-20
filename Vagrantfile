# -*- mode: ruby -*-
# vi: set ft=ruby :

vagrant_private_key_file="vagrant_key"

Vagrant.configure(2) do |config|
    config.vm.box = "debian/jessie64"
    #config.vm.box = "pelagibuild"

    # Deploy a private key used to clone gits from pelagicore.net
    config.vm.provision "file", source: vagrant_private_key_file, destination: "/home/vagrant/.ssh/id_rsa"

    config.vm.provision "shell", inline: <<-SHELL
        # Workaround for some bad network stacks
        ping google.com 1> /dev/null 2>&1 &
    SHELL

    # Install dependencies via apt
    config.vm.provision "shell", inline: <<-SHELL

        alias apt-get="apt-get -y --force-yes"
        # Several packages need to be very recent, so use debian testing
        sed -i 's/jessie/testing/g' /etc/apt/sources.list
        apt-get update
        DEBIAN_FRONTEND=noninteractive apt-get -o Dpkg::Options::="--force-confnew" --force-yes -fuy dist-upgrade
        apt-get -y --force-yes autoremove

        # Common dependencies
        apt-get install -y --force-yes git cmake build-essential pkg-config

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
        rm -rf ivi-logging
        git clone https://github.com/Pelagicore/ivi-logging.git
        mkdir ivi-logging/build
        cd ivi-logging/build
        cmake ..
        make && sudo make install

        cd

        # Pelagicore-utils
        rm -rf pelagicore-utils
        git clone git@git.pelagicore.net:development-tools/pelagicore-utils.git
        mkdir pelagicore-utils/build
        cd pelagicore-utils/build
        cmake ..
        make && sudo make install

        cd

        #ivi-mainloop
        rm -rf ivi-main-loop
        git clone https://github.com/Pelagicore/ivi-main-loop.git
        mkdir ivi-main-loop/build
        cd ivi-main-loop/build
        cmake ..
        make && sudo make install

        # jsonparser
        rm -rf jsonparser
        git clone git@git.pelagicore.net:pelagicore-utils/jsonparser.git
        mkdir jsonparser/build
        cd jsonparser/build
        cmake ..
        make && sudo make install
    SHELL

    # Install project
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        sudo rm -rf pelagicontain
        #git clone git@git.pelagicore.net:application-management/pelagicontain.git
        git clone git@git.pelagicore.net:oandreasson/pelagicontain.git
        mkdir pelagicontain/build
        cd pelagicontain/build
        cmake ..
        make && sudo make install
    SHELL

    # Run an example (note, running as root)
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        cd pelagicontain/examples/simple
        cmake .
        make
        sudo ./launch.sh -b session
    SHELL
end
