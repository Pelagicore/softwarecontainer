
# Google Mock add (and fetch) macro
macro(add_gmock)
    set(ZIPNAME release-1.8.0)
    set(MOCKDIR googletest-${ZIPNAME})

    if(NOT IS_DIRECTORY ${CMAKE_BINARY_DIR}/${MOCKDIR})
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${ZIPNAME}.zip)
            message(STATUS "Downloading googletest...")
            execute_process(COMMAND wget -nv -P ${CMAKE_BINARY_DIR} https://github.com/google/googletest/archive/${ZIPNAME}.zip)
        endif()
        message(STATUS "Extracting ${ZIPNAME}...")
        execute_process(COMMAND unzip -q ${CMAKE_BINARY_DIR}/${ZIPNAME}.zip -d ${CMAKE_BINARY_DIR})
    endif()

    if(IS_DIRECTORY ${CMAKE_BINARY_DIR}/${MOCKDIR})
        add_subdirectory(${CMAKE_BINARY_DIR}/${MOCKDIR})
    else()
        message(FATAL_ERROR "No ${MOCKDIR} found, can't add googletest")
    endif()
endmacro()

