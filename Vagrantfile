# -*- mode: ruby -*-
# vi: set ft=ruby :

vagrant_private_key_file="vagrant_key"

Vagrant.configure(2) do |config|
    config.vm.box = "debian/jessie64"
    #config.vm.box = "pelagibuild"

    # Deploy a private key used to clone gits from pelagicore.net
    config.vm.provision "file", source: vagrant_private_key_file, destination: "/home/vagrant/.ssh/id_rsa"

    # Workaround for some bad network stacks
    config.vm.provision "shell", privileged: false, path: "cookbook/utils/keepalive.sh" 

    # Use apt-cacher on main server
    config.vm.provision "shell", 
        args: ['10.8.36.16'],
        path: "cookbook/system-config/apt-cacher.sh" 

    # Upgrade machine to testing distro & install build dependencies
    config.vm.provision "shell", path: "cookbook/deps/testing-upgrade.sh"
    config.vm.provision "shell", path: "cookbook/deps/common-build-dependencies.sh"

    # Add known hosts
    config.vm.provision "shell", privileged: false, path: "cookbook/system-config/pelagicore-ssh-conf.sh"

    # Install dependencies via git
    config.vm.provision "shell", privileged: false, 
        args: ["ivi-logging", "https://github.com/Pelagicore/ivi-logging.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        args: ["pelagicore-utils", "git@git.pelagicore.net:development-tools/pelagicore-utils.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        args: ["ivi-main-loop", "https://github.com/Pelagicore/ivi-main-loop.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        args: ["jsonparser", "git@git.pelagicore.net:pelagicore-utils/jsonparser.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        args: ["dbus-proxy", "git@git.pelagicore.net:application-management/dbus-proxy.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    # Build and install project
    config.vm.provision "shell", privileged: false, 
        args: ["pelagicontain", "-DENABLE_DOC=1 -DENABLE_TEST=ON -DENABLE_COVERAGE=1 -DENABLE_SYSTEMD=1"],
        path: "cookbook/build/cmake-builder.sh"

    # Build documentation and run unit tests
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        cd pelagicontain/build
        make doc
    SHELL

    config.vm.provision "shell", inline: <<-SHELL
        cd pelagicontain/build/libpelagicontain/unit-test
        eval $(dbus-launch --sh-syntax)
        echo "D-Bus per-session daemon address is: $DBUS_SESSION_BUS_ADDRESS"

        ./pelagicontainLibTest --gtest_filter=-PelagicontainApp.FileGatewayReadOnly:PelagicontainApp.TestPulseAudioEnabled

    SHELL

    # Run an example (note, running as root)
    config.vm.provision "shell", privileged: false, inline: <<-SHELL
        cd pelagicontain/examples/simple
        cmake .
        make
        sudo ./launch.sh -b session
    SHELL

    # clang analysis of the code
    config.vm.provision "shell", privileged: false, 
        args: ["clang", "-DENABLE_DOC=1 -DBUILD_TESTS=ON -DENABLE_COVERAGE=1"],
        path: "cookbook/build/clang-code-analysis.sh"

end
