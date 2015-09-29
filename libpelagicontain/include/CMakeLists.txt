
set(HEADERS
    libpelagicontain.h
    pelagicontain.h
    container.h
    gateway.h
    ${PROJECT_BINARY_DIR}/pelagicontain-config.h
)

install(FILES ${HEADERS} DESTINATION include/pelagicontain)
