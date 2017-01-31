#!/usr/bin/groovy

/*
 * Copyright (C) 2016-2017 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

// Runs a shell command in the vagrant vm
def runInVagrant = { String workspace, String command ->
    sh "cd ${workspace} && vagrant ssh -c '${command}'"
}

def shutdownVagrant = {
    // In case the destroy step fails (happens sometimes with hung up ssh connections)
    // we retry a couple of times to make sure it shuts down and is destroyed.
    retry(5) {
        sh "cd ${workspace} && vagrant destroy -f"
    }
}

node {
    String workspace = pwd()
    try {
        // Store the directory we are executed in as our workspace.
        // These are the build parameters we want to use
        String buildParams = "-DENABLE_TEST=ON -DENABLE_COVERAGE=ON "
        buildParams       += "-DENABLE_USER_DOC=ON -DENABLE_API_DOC=ON "
        buildParams       += "-DENABLE_SYSTEMD=ON -DENABLE_PROFILING=ON "
        buildParams       += "-DCMAKE_INSTALL_PREFIX=/usr"

        // Stages are subtasks that will be shown as subsections of the finiished build in Jenkins.
        stage('Download') {
            // Checkout the git repository and refspec pointed to by jenkins
            checkout scm
            // Update the submodules in the repository.
            sh 'git submodule update --init'
        }

        stage('StartVM') {
            // Calculate available amount of RAM (excluding Swap)
            String gigsramStr = sh (
                script: 'free -g | grep "Mem: " | tail -n1 | awk \'{ print $2 }\'',
                returnStdout: true
            )
            int gigsram = gigsramStr.trim() as Integer
            // Only use half of the available memory if it is more than 2GB
            if (gigsram >= 2) {
                gigsram = gigsram / 2
                println "Will set VAGRANT_RAM to ${gigsram}"
            }

            // Start the machine (destroy it if present) and provision it
            shutdownVagrant()
            withEnv(["VAGRANT_RAM=${gigsram}",
                     "APT_CACHE_SERVER=10.8.36.16"]) {
                sh "cd ${workspace} && vagrant up"
            }
        }

        stage('Build') {
            runInVagrant(workspace, "sh ./softwarecontainer/cookbook/build/cmake-builder.sh \
                                     softwarecontainer \"${buildParams}\" -DENABLE_EXAMPLES=ON")
        }

        // TODO: Haven't figured out how to make the parallel jobs run on the same slave/agent
        // or if it is possible. Very annoying. Let's run sequentially in a single slave for now.
        stage('Clang') {
            runInVagrant(workspace, "sh ./softwarecontainer/cookbook/build/clang-code-analysis.sh \
                                     softwarecontainer clang \"${buildParams}\"")
        }

        stage('User documentation') {
            runInVagrant(workspace, 'cd softwarecontainer/build && make user-doc')
        }

        stage('API documentation') {
            runInVagrant(workspace, 'cd softwarecontainer/build && make api-doc')
        }

        stage('UnitTest') {
            runInVagrant(workspace, "cd softwarecontainer && ./build/run-unit-tests.py")
        }

        stage('ComponentTest') {
            // We run it in this way to avoid getting (unreachable) paths
            runInVagrant(workspace, "cd softwarecontainer && sudo ./build/run-component-tests.py")
        }

        stage('ServiceTest') {
            runInVagrant(workspace, "cd softwarecontainer/servicetest && sudo ./run-tests.sh")
        }

        stage('Coverage') {
            // We run it in this way to avoid getting (unreachable) paths
            runInVagrant(workspace, "cd softwarecontainer && sudo make -C build lcov")
        }

        stage('Examples') {
            runInVagrant(workspace, "cd softwarecontainer/examples && sudo ./run-tests.sh")
        }
        // END TODO

        stage('Artifacts') {
            // Store the artifacts of the entire build
            archive "**/*"

            // Save the coverage report for unit tests
            publishHTML (target: [
                allowMissing: false,
                alwaysLinkToLastBuild: false,
                keepAll: true,
                reportDir: 'build/CoverageUnitTest',
                reportFiles: 'index.html',
                reportName: 'Coverage report'
            ])

            publishHTML (target: [
                allowMissing: false,
                alwaysLinkToLastBuild: false,
                keepAll: true,
                reportDir: 'clang/scan_build_output/report',
                reportFiles: 'index.html',
                reportName: 'Clang report'
            ])

            // Store the unit test results and graph them
            step([$class: 'JUnitResultArchiver', testResults: '**/*_unittest_result.xml'])
            // Store the component test results and graph them
            step([$class: 'JUnitResultArchiver', testResults: '**/*_componenttest_result.xml'])
            // Store the service test results and graph them
            step([$class: 'JUnitResultArchiver', testResults: '**/*_servicetest_result.xml'])
        }
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

    finally {
        // Always try to shut down the machine
        shutdownVagrant()
    }
}

