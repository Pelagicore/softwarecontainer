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
def runInVagrant = { String command ->
    sh "cd ${env.WORKSPACE} && vagrant ssh -c 'cd softwarecontainer && ${command}'"
}

def shutdownVagrant = {
    // In case the destroy step fails (happens sometimes with hung up ssh connections)
    // we retry a couple of times to make sure it shuts down and is destroyed.
    retry(5) {
        sh "cd ${env.WORKSPACE} && vagrant destroy -f"
    }
}

node {
    try {

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
                sh "cd ${env.WORKSPACE} && vagrant up"
            }
        }

        // Configure the software with cmake
        stage('Configure') {
            String buildParams = "-DENABLE_COVERAGE=ON "
            buildParams       += "-DENABLE_USER_DOC=ON -DENABLE_API_DOC=ON "
            buildParams       += "-DENABLE_SYSTEMD=ON -DENABLE_PROFILING=ON "
            buildParams       += "-DENABLE_TEST=ON -DENABLE_EXAMPLES=ON"

            // Remove old build directory
            runInVagrant("rm -rf build")
            // Run cmake
            runInVagrant("cmake -H. -Bbuild ${buildParams}")
        }

        // We build the docs first because they are part of ALL
        stage('API documentation') {
            runInVagrant("cd build && make api-doc")
        }

        // We build the docs first because they are part of ALL
        stage('User documentation') {
            runInVagrant("cd build && make user-doc")
        }

        // Build the rest of the projekt
        stage('Build') {
            runInVagrant("cd build && make")
        }

        stage('Install') {
            runInVagrant("cd build && sudo make install")
        }

        stage('UnitTest') {
            // TODO: We should not run these tests as root, but while waiting for a fix in the
            //       agent implementation, we currently have to.
            runInVagrant("sudo ./build/run-unit-tests.py")
        }

        stage('ComponentTest') {
            // We run it in this way to avoid getting (unreachable) paths
            runInVagrant("sudo ./build/run-component-tests.py")
        }

        stage('ServiceTest') {
            runInVagrant("cd servicetest && sudo ./run-tests.sh")
        }

        stage('Clang') {
            // Most build parameters default to off, but not tests. We don't want static
            // analysis for our tests.
            String buildParams = "-DENABLE_TEST=OFF"
            runInVagrant("sh ./cookbook/build/clang-code-analysis.sh . clang ${buildParams}")
        }

        stage('Coverage') {
            // We run it in this way to avoid getting (unreachable) paths
            runInVagrant("sudo make -C build lcov")
        }

        stage('Examples') {
            runInVagrant("cd examples && sudo ./run-tests.sh")
            runInVagrant("cd doc && sudo ./run_examples.sh")
        }

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

