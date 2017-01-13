#
# Copyright (C) 2016-2017 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE
#

# Macro for making it easy to use gcov/lcov
macro(add_coverage outputDir testCommand)
    #
    # First, check if we have an lcov target, and if not - create one.
    # This also checks for the required tools to be able to add an lcov target.
    #
    if (NOT TARGET lcov)
        find_program(LCOV lcov)
        find_program(GENHTML genhtml)

        if (LCOV AND GENHTML)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
            add_custom_target(lcov)
        else()
            message(FATAL_ERROR "Please install lcov and genhtml to enable the lcov target")
        endif()
    endif()

    #
    # If we already have an lcov target, we add our sub-targets here
    #

    set(COV_DIR ${outputDir})
    add_custom_target(lcov_${COV_DIR})
    add_dependencies(lcov lcov_${COV_DIR})

    add_custom_command(
        TARGET lcov_${COV_DIR}
        COMMAND rm -rf ${COV_DIR}
        COMMAND mkdir -p ${COV_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Run baseline coverage file, then remove unwanted files from baseline data
    add_custom_command(
        TARGET lcov_${COV_DIR}
        # Create baseline
        COMMAND ${LCOV} --capture --initial --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        # Remove unwanted files from baseline data
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"build*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"examples*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
        COMMENT "Getting lcov baseline data"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )


    # Run the actual tests and stuff
    add_custom_command(
        TARGET lcov_${COV_DIR}

        # Run tests
        COMMAND ${testCommand}
        # Capture coverage data after tests have run
        COMMAND ${LCOV} --capture --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        # Remove unwanted files from captured data
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"build*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMAND ${LCOV} --remove ${COV_DIR}/testrun.info \"examples*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/testrun.info
        COMMENT "Running tests (${testCommand}) and lcov"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_command(
        TARGET lcov_${COV_DIR}
        # Combine baseline data and coverage data for accurate coverage stats
        COMMAND ${LCOV} --add-tracefile ${COV_DIR}/baseline.info --add-tracefile ${COV_DIR}/testrun.info --output-file ${COV_DIR}/final.info
        # Generate HTML report
        COMMAND genhtml --prefix ${CMAKE_SOURCE_DIR} ${COV_DIR}/final.info --output-directory ${COV_DIR}/
        COMMENT "Combining baseline data with data from lcov run"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endmacro(add_coverage)

