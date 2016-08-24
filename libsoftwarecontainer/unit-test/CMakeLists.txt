
# Copyright (C) 2016 Pelagicore AB
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



set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test-data)
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

set(TEST_LIBRARY_DEPENDENCIES 
    ${DBusCpp_LIBRARIES}
    ${Glibmm_LIBRARIES}
    ${LXC_LIBRARIES}
    ${IVIMainLoop_LIBRARIES}
    ${Jansson_LIBRARIES}
    ${IVILogging_LIBRARIES}
    softwarecontainerLib
)

include_directories(${LIBSOFTWARECONTAINER_DIR}/include)
link_directories(
    ${DEPENDENCIES_LIBRARY_DIRS}
    ${LOGGING_LIBRARY_DIRS}
)

set(TEST_FILES
    main.cpp
    softwarecontainer_test.cpp
    softwarecontainerlib_unittest.cpp
    cgroupsgateway_unittest.cpp
    filegateway_unittest.cpp
    envgateway_unittest.cpp
    dbusgateway_unittest.cpp
)

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
			cat $TMP_FILE | sed -r 's/^\\[\\s+OK\\s+\\] (.*) \\([0-9]+\\sms\\)$/PASS: \\1 /' | sed -r 's/^\\[\\s+FAILED\\s+\\] (.*) \\([0-9]+\\sms\\)$/FAIL: \\1 /' | awk '{if ($1 == \"PASS:\" || $1 == \"FAIL:\") {print $0}}'
		fi
	fi
done
")

	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/run-ptest" DESTINATION ${TESTS_INSTALLATION_PATH}
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

endmacro()



add_gtest_test(softwarecontainerLibTest
    "${TEST_FILES}"
    "${TEST_LIBRARY_DEPENDENCIES}"
)
