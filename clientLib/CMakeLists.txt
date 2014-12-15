set(HEADERS 
    pelagicontain-lib.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pelagicontain.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/container.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/gateway.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pelagicontaintodbusadapter.h
    ${PROJECT_SOURCE_DIR}/common/pelagicontain-common.h
    ${PROJECT_SOURCE_DIR}/common/pelagicontain-log.h
    ${PROJECT_BINARY_DIR}/pelagicontain-config.h
)

install(FILES ${HEADERS} DESTINATION include/pelagicontain)
