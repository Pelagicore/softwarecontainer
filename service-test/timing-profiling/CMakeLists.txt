CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_EXECUTABLE(simple simple.c)
INSTALL(TARGETS simple
        RUNTIME DESTINATION share/pelagicontain/examples/simple)

INSTALL(FILES launch.sh DESTINATION share/pelagicontain/examples/simple)
