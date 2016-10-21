macro(generate_dbuscpp_hfile xmlInterfaceFile h)

    set(DBUS_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/GeneratedDBusCode)
    file(MAKE_DIRECTORY ${DBUS_GENERATED_DIR})

    include_directories(${DBUS_GENERATED_DIR})

    set(GENERATED_PROXY_FILENAME ${DBUS_GENERATED_DIR}/${h}_dbuscpp_proxy.h)
    set(GENERATED_ADAPTOR_FILENAME ${DBUS_GENERATED_DIR}/${h}_dbuscpp_adaptor.h)

    message("Creating DBusCpp proxy and stubs : ${GENERATED_PROXY_FILENAME} / ${GENERATED_ADAPTOR_FILENAME}")

    add_custom_command(
        OUTPUT ${GENERATED_PROXY_FILENAME} ${GENERATED_ADAPTOR_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --proxy=${GENERATED_PROXY_FILENAME}
        COMMAND dbusxx-xml2cpp ${xmlInterfaceFile} --adaptor=${GENERATED_ADAPTOR_FILENAME}
        DEPENDS ${xmlInterfaceFile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(dbus_cpp_${h} ALL DEPENDS ${GENERATED_PROXY_FILENAME} ${GENERATED_ADAPTOR_FILENAME})

endmacro()
