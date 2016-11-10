macro(add_gtest_test testName files libraries)

    if ((NOT TARGET gtest) OR (NOT TARGET gmock))
        message(FATAL_ERROR "No gtest or gmock targets found, can't add tests")
    else()
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

        add_test(
            NAME ${testName}
            COMMAND ${testName}
        )
    endif()

endmacro()
