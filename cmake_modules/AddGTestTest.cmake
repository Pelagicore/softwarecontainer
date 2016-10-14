macro(add_gtest_test testName files libraries)

        if(NOT DEFINED TESTS_INSTALLATION_PATH)
                set(TESTS_INSTALLATION_PATH "lib/${PROJECT_NAME}/ptest")
        endif()

        message("Tests will be installed in : ${TESTS_INSTALLATION_PATH}")

        if (DEFINED $ENV{GTEST_DIR})
                set(GTEST_DIR $ENV{GTEST_DIR})
        endif()

        message("Google test found in ${GTEST_DIR}")

        include_directories(${GTEST_DIR}/include)
        link_directories(${GTEST_DIR}/lib/.libs)

        add_executable(${testName}
                ${files}
        )

        target_link_libraries(${testName}
                ${libraries}
                gtest
                gtest_main
                gmock
                gmock_main
        )

        INSTALL(TARGETS ${testName}
          DESTINATION ${TESTS_INSTALLATION_PATH}
        )

        add_test(
            NAME ${testName}
            COMMAND ${testName}
        )

        file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run-ptest "#!/bin/bash
FOLDER=`dirname $0`
for filename in $FOLDER/*; do
        TEST_FILE=$filename
        if [ \"`basename $filename`\" != \"run-ptest\" ]; then
                if [[ -x $TEST_FILE ]]; then
#			echo Starting $TEST_FILE
                        TMP_FILE=`mktemp`
                        # Run the test and convert its output to the ptest format
                        $TEST_FILE >> $TMP_FILE
                        cat $TMP_FILE | sed -r 's/^\\[\\s+OK\\s+\\] (.*) \\([0-9]+\\sms\\)$/PASS: \\1 /' \
                            | sed -r 's/^\\[\\s+FAILED\\s+\\] (.*) \\([0-9]+\\sms\\)$/FAIL: \\1 /' \
                            | awk '{if ($1 == \"PASS:\" || $1 == \"FAIL:\") {print $0}}'
                fi
        fi
done
")

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/run-ptest" DESTINATION ${TESTS_INSTALLATION_PATH}
                PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

endmacro()
