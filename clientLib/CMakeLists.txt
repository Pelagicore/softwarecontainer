set(HEADERS 
    pelagicontain-lib.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/paminterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pamproxy.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pamabstractinterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pelagicontain.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/container.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/gateway.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/controllerinterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/controllerabstractinterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/mainloopabstractinterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/pelagicontaintodbusadapter.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/dbusadaptor.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/systemcallinterface.h
    ${PROJECT_SOURCE_DIR}/pelagicontain/src/systemcallabstractinterface.h
    ${PROJECT_SOURCE_DIR}/common/pelagicontain-common.h
    ${PROJECT_SOURCE_DIR}/common/pelagicontain-log.h
    ${PROJECT_BINARY_DIR}/pelagicontain-config.h
)

install(FILES ${HEADERS} DESTINATION include/pelagicontain)
