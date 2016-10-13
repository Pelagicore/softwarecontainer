#!/usr/bin/groovy

def runInVagrant = { String workspace, String command ->
    sh "cd ${workspace} && \
        vagrant ssh -c '${command}' \
        "
}

node {
    // Store the directory we are executed in as our workspace.
    workspace = pwd()
    stage 'Download'
    // Checkout the git repository and refspec pointed to by jenkins
    checkout scm
    // Update the submodules in the repository.
    sh 'git submodule update --init'

    stage 'Build'
    // STart the machine and provision it
    // TODO This should be split into two parts, one for provisioning and one for actually
    // building softwarecontainer
    sh "cd ${workspace} && vagrant up --provision"


    // TODO: Haven't figured out how to make the parallel jobs run on the same slave/agent
    // or if it is possible. Very annoying. Let's run sequentially in a single slave for now.
    stage 'Clang'
    runInVagrant(workspace, 'sh ./softwarecontainer/cookbook/build/clang-code-analysis.sh \
            softwarecontainer clang \"-DENABLE_DOC=1 -DENABLE_TEST=ON -DENABLE_COVERAGE=1\"')
    stage 'Documentation'
    runInVagrant(workspace, 'cd softwarecontainer/build && make doc')
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

    stage 'Shutdown'
    // Shutdown the machine
    // TODO: it is not destroyed... maybe it should be
    sh "cd ${workspace} && vagrant halt || true"
}

