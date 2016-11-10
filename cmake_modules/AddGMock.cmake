
# Google Mock add (and fetch) macro
macro(add_gmock)
    set(ZIPNAME release-1.8.0)
    set(ZIPFILE ${CMAKE_CURRENT_BINARY_DIR}/${ZIPNAME}.zip)
    set(MOCKDIR ${CMAKE_CURRENT_BINARY_DIR}/googletest-${ZIPNAME})

    if(NOT IS_DIRECTORY ${MOCKDIR})
        if(NOT EXISTS ${ZIPFILE})
            message(STATUS "Downloading googletest...")
            execute_process(COMMAND wget -nv -P ${CMAKE_CURRENT_BINARY_DIR} https://github.com/google/googletest/archive/${ZIPNAME}.zip)
        endif()
        message(STATUS "Extracting ${ZIPNAME}...")
        execute_process(COMMAND unzip -q ${ZIPFILE} -d ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if(IS_DIRECTORY ${MOCKDIR})
        add_subdirectory(${MOCKDIR} EXCLUDE_FROM_ALL)
    else()
        message(FATAL_ERROR "No ${MOCKDIR} found, can't add googletest")
    endif()
endmacro()

