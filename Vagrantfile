# -*- mode: ruby -*-
# vi: set ft=ruby :

vagrant_private_key_file="vagrant_key"

ram = 4 #GB
cpus = 3

Vagrant.configure(2) do |config|
    config.vm.box = "debian/contrib-jessie64"
    config.vm.provider "virtualbox" do |vb|
        vb.customize [ "guestproperty", "set", :id, 
                       "/VirtualBox/GuestAdd/VBoxService/--timesync-set-threshold", 200 ]
        #vb.memory = ram * 1024
        #vb.cpus = cpus
    end

    # Sync the repo root with this path in the VM
    config.vm.synced_folder "./", "/home/vagrant/softwarecontainer/", create: true

    # Deploy a private key used to clone gits from pelagicore.net
    config.vm.provision "file", source: vagrant_private_key_file, 
        destination: "/home/vagrant/.ssh/id_rsa"

    # Workaround for some bad network stacks
    config.vm.provision "shell", privileged: false, path: "cookbook/utils/keepalive.sh" 

    # Use apt-cacher on main server
    config.vm.provision "shell", 
        args: ['10.8.36.16'],
        path: "cookbook/system-config/apt-cacher.sh" 

    # Select the best apt mirror
    config.vm.provision "shell", path: "cookbook/system-config/select-apt-mirror.sh"

    # Upgrade machine to testing distro & install build dependencies
    config.vm.provision "shell", path: "cookbook/deps/testing-upgrade.sh"
    config.vm.provision "shell", path: "cookbook/deps/common-build-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/common-run-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/wayland-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/pulseaudio-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/sphinx-dependencies.sh"
    config.vm.provision "shell", path: "cookbook/deps/pytest-and-dbus-testing-dependencies.sh"

    # Add known hosts
    config.vm.provision "shell", privileged: false,
        path: "cookbook/system-config/ssh-keyscan-conf.sh"

    # Install dependencies via git
    config.vm.provision "shell", privileged: false, 
        args: ["dlt-daemon", "http://git.projects.genivi.org/dlt-daemon.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        #args: ["ivi-logging", "https://github.com/Pelagicore/ivi-logging.git", "-DENABLE_DLT_BACKEND=1"],
        args: ["ivi-logging", "https://github.com/Pelagicore/ivi-logging.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    config.vm.provision "shell", privileged: false, 
        args: ["dbus-proxy", "git@git.pelagicore.net:application-management/dbus-proxy.git"],
        path: "cookbook/build/cmake-git-builder.sh"

    # Build and install project
    config.vm.provision "shell", privileged: false, 
        args: ["softwarecontainer", "-DENABLE_DOC=1 -DENABLE_TEST=ON -DENABLE_COVERAGE=1 -DENABLE_SYSTEMD=1 -DENABLE_PROFILING=1"],
        path: "cookbook/build/cmake-builder.sh"

    if ENV['CI_BUILD'] then
        # clang analysis of the code
        config.vm.provision "shell", privileged: false,
            args: ["softwarecontainer", "clang", "-DENABLE_DOC=1 -DENABLE_TEST=ON -DENABLE_COVERAGE=1"],
            path: "cookbook/build/clang-code-analysis.sh"

        config.vm.provision "shell", inline: <<-SHELL
            cd softwarecontainer
            sudo su -c ./run-all-tests.sh
        SHELL
    end
end
