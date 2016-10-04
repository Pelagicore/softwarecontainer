#!/usr/bin/groovy

def runInVagrant = { String workspace, String command ->
    sh "cd ${workspace} && \
        vagrant ssh -c '${command}' \
        "
}

node {
    workspace = pwd()
    stage 'Download'
    git branch: 'master', url: 'https://github.com/Pelagicore/softwarecontainer.git'
    sh 'git submodule update --init'

    stage 'Build'
    sh "cd ${workspace} && vagrant up --provision"

    stage 'Clang'
    runInVagrant(workspace, 'sh ./softwarecontainer/cookbook/build/clang-code-analysis.sh \
            softwarecontainer clang \"-DENABLE_DOC=1 -DENABLE_TEST=ON -DENABLE_COVERAGE=1\"')

// TBD: Haven't figured out how to make the parallel jobs run on the same slave/agent 
// or if it is possible. Very annoying. Let's run sequentially in a single slave for now.
//    parallel ('Documentation': {
    stage 'Documentation'
    runInVagrant(workspace, 'cd softwarecontainer/build && make doc')

//    }, 'UnitTest':{
    stage 'UnitTest'
    runInVagrant(workspace, "cd softwarecontainer/build && sudo ./run-tests.sh")
//    }, 'ServiceTest':{
    stage 'ServiceTest'
    runInVagrant(workspace, "cd softwarecontainer/servicetest && sudo ./run-tests.sh")
//    })
    stage 'Examples'
    runInVagrant(workspace, "cd softwarecontainer/examples && sudo ./run-tests.sh")

    stage 'Shutdown'
    sh 'cd ${workspace} && vagrant halt || true'
}

