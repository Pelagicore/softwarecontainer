# Macro for making it easy to use gcov/lcov
macro(add_coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
        add_custom_target(lcov)
        add_custom_command(
            TARGET lcov
            COMMAND mkdir -p coverage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        # Run baseline coverage file, then remove unwanted files from baseline data
        add_custom_command(
            TARGET lcov
            # Create baseline
            COMMAND ${LCOV} --capture --initial --directory . --base-directory . --output-file coverage/baseline.info
            # Remove unwanted files from baseline data
            COMMAND ${LCOV} --remove coverage/baseline.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
            COMMAND ${LCOV} --remove coverage/baseline.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
            COMMAND ${LCOV} --remove coverage/baseline.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
            COMMAND ${LCOV} --remove coverage/baseline.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file coverage/baseline.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        # Run the actual tests and stuff
        add_custom_command(
            TARGET lcov
            # Run tests
            COMMAND make test
            # Capture coverage data after tests have run
            COMMAND ${LCOV} --capture --directory . --base-directory . --output-file coverage/testrun.info
            # Remove unwanted files from captured data
            COMMAND ${LCOV} --remove coverage/testrun.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
            COMMAND ${LCOV} --remove coverage/testrun.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
            COMMAND ${LCOV} --remove coverage/testrun.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
            COMMAND ${LCOV} --remove coverage/testrun.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file coverage/testrun.info
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_command(
            TARGET lcov
            # Combine baseline data and coverage data for accurate coverage stats
            COMMAND ${LCOV} --add-tracefile coverage/baseline.info --add-tracefile coverage/testrun.info --output-file coverage/final.info
            # Generate HTML report
            COMMAND genhtml --no-prefix coverage/final.info --output-directory coverage/
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else(LCOV AND GENHTML)
        message(WARNING "Please install lcov and genhtml to enable the lcov target (apt-get install lcov)" )
    endif(LCOV AND GENHTML)
    message("Code coverage enabled")
endmacro(add_coverage)


