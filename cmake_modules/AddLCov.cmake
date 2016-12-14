# Macro for making it easy to use gcov/lcov
macro(add_coverage testCommand)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)

        set(COV_DIR "coverage")
        add_custom_target(lcov)
        add_custom_command(
            TARGET lcov
            COMMAND mkdir -p ${COV_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        # Run baseline coverage file, then remove unwanted files from baseline data
        add_custom_command(
            TARGET lcov
            # Create baseline
            COMMAND ${LCOV} --capture --initial --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            # Remove unwanted files from baseline data
            COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"/usr*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"gmock*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"gtest*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"unit-test*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            COMMAND ${LCOV} --remove ${COV_DIR}/baseline.info \"build*\" --no-checksum --directory . --base-directory . --output-file ${COV_DIR}/baseline.info
            COMMENT "Getting lcov baseline data"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )


        # Run the actual tests and stuff
        add_custom_command(
            TARGET lcov

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
            COMMENT "Running tests (${testCommand}) and lcov"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_command(
            TARGET lcov
            # Combine baseline data and coverage data for accurate coverage stats
            COMMAND ${LCOV} --add-tracefile ${COV_DIR}/baseline.info --add-tracefile ${COV_DIR}/testrun.info --output-file ${COV_DIR}/final.info
            # Generate HTML report
            COMMAND genhtml --prefix ${CMAKE_SOURCE_DIR} ${COV_DIR}/final.info --output-directory ${COV_DIR}/
            COMMENT "Combining baseline data with data from lcov run"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else(LCOV AND GENHTML)
        message(WARNING "Please install lcov and genhtml to enable the lcov target")
    endif(LCOV AND GENHTML)
endmacro(add_coverage)

