cmake_minimum_required(VERSION 2.6)

project(containedapp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../../../cmake_modules)

find_package(DBusCpp REQUIRED)

include_directories (${DBUS_INCLUDE_DIR} ${DBUSCPP_INCLUDE_DIR})

add_executable(containedapp
	containedapp.cpp
)

target_link_libraries (containedapp
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
	${DBUS_LIBRARIES} ${DBUSCPP_LIBRARIES}
)

install(TARGETS containedapp RUNTIME DESTINATION bin)
