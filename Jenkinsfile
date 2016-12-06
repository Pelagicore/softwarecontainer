#!/usr/bin/groovy

// Runs a shell command in the vagrant vm
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

        // Stages are subtasks that will be shown as subsections of the finiished build in Jenkins.
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
            runInVagrant(workspace, "cd softwarecontainer/build && sudo ./run-tests.py")
        stage 'ServiceTest'
            runInVagrant(workspace, "cd softwarecontainer/servicetest && sudo ./run-tests.sh")
        stage 'Examples'
            runInVagrant(workspace, "cd softwarecontainer/examples && sudo ./run-tests.sh")
        // END TODO

        stage 'Artifacts'
            // Store the artifacts of the entire build
            archive "**/*"

            // Store the test results and graph them
            // TODO: There is an issue with the build directory ending up as (unreachable)
            step([$class: 'JUnitResultArchiver', testResults: '**/*_unittest_result.xml'])
            // Store the service test results and graph them
            step([$class: 'JUnitResultArchiver', testResults: '**/*_servicetest_result.xml'])
    }

    catch(err) {
        // Do not add a stage here.
        // When "stage" commands are run in a different order than the previous run
        // the history is hidden since the rendering plugin assumes that the system has changed and
        // that the old runs are irrelevant. As such adding a stage at this point will trigger a
        // "change of the system" each time a run fails.
        println "Something went wrong!"
        currentBuild.result = "FAILURE"
    }

    // Always try to shut down the machine
    // Shutdown the machine
    sh "cd ${workspace} && vagrant halt || true"
}

