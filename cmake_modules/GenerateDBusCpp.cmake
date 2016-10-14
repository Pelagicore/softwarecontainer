macro(generate_dbuscpp_hfile xmlInterfaceFile h)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(GENERATED_PROXY_FILENAME ${h}_dbuscpp_proxy.h)
    set(GENERATED_ADAPTOR_FILENAME ${h}_dbuscpp_adaptor.h)

    message("Creating DBusCpp proxy and stubs : ${GENERATED_PROXY_FILENAME} / ${GENERATED_ADAPTOR_FILENAME}")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_PROXY_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_ADAPTOR_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --proxy=${GENERATED_PROXY_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --adaptor=${GENERATED_ADAPTOR_FILENAME}
        DEPENDS ${xmlInterfaceFile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(dbus_cpp_${h} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_PROXY_FILENAME} ${CMAKE_CURRENT_BINARY_DIR}/${GENERATED_ADAPTOR_FILENAME})
endmacro()
