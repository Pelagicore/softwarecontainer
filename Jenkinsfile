#!/usr/bin/groovy

def runInVagrant = { String workspace, String command ->
    sh "cd ${workspace} && vagrant ssh -c '${command}'"
}

node {
    String workspace = pwd()
    try {
        // Store the directory we are executed in as our workspace.
        // These are the build parameters we want to use
        String buildParams = "-DENABLE_TEST=ON -DENABLE_COVERAGE=ON "
        buildParams       += "-DENABLE_USER_DOC=ON -DENABLE_API_DOC=ON "
        buildParams       += "-DENABLE_SYSTEMD=ON -DENABLE_PROFILING=ON "
        buildParams       += "-DENABLE_EXAMPLES=ON -DCMAKE_INSTALL_PREFIX=/usr"

        stage 'Download'
            // Checkout the git repository and refspec pointed to by jenkins
            checkout scm
            // Update the submodules in the repository.
            sh 'git submodule update --init'

        stage 'StartVM'
            // Start the machine (destroy it if present) and provision it
            sh "cd ${workspace} && vagrant destroy -f || true"
            sh "cd ${workspace} && APT_CACHE_SERVER=\"10.8.36.16\" vagrant up"

        stage 'Build'
            runInVagrant(workspace, "sh ./softwarecontainer/cookbook/build/cmake-builder.sh \
                                     softwarecontainer \"${buildParams}\"")

        // TODO: Haven't figured out how to make the parallel jobs run on the same slave/agent
        // or if it is possible. Very annoying. Let's run sequentially in a single slave for now.
        stage 'Clang'
            runInVagrant(workspace, "sh ./softwarecontainer/cookbook/build/clang-code-analysis.sh \
                                     softwarecontainer clang \"${buildParams}\"")
        stage 'User documentation'
            runInVagrant(workspace, 'cd softwarecontainer/build && make user-doc')
        stage 'API documentation'
            runInVagrant(workspace, 'cd softwarecontainer/build && make api-doc')
        stage 'UnitTest'
            runInVagrant(workspace, "cd softwarecontainer/build && sudo ./run-tests.sh")
        stage 'ServiceTest'
            runInVagrant(workspace, "cd softwarecontainer/servicetest && sudo ./run-tests.sh")
        stage 'Examples'
            runInVagrant(workspace, "cd softwarecontainer/examples && sudo ./run-tests.sh")
        // END TODO

        stage 'Artifacts'
            // Store the artifacts of the entire build
            archive "**/*"
            // Store the unit test results and graph it
            // NOTE: the paths can not be absolute, it will not be accepted for some reason
            step([$class: 'JUnitResultArchiver', testResults: 'build/*test.xml'])
            step([$class: 'JUnitResultArchiver', testResults: 'servicetest/*test.xml'])

    }

    catch(err) {
        println "Something went wrong!"
        currentBuild.result = "FAILURE"
    }

    // Always try to shut down the machine
    // Shutdown the machine
    sh "cd ${workspace} && vagrant halt || true"
}

