
SET(CONTROLLER_DIR ${CMAKE_INSTALL_PREFIX}/bin)

add_definitions(-DCONTROLLER_DIR="${CONTROLLER_DIR}")

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS})

add_executable(controller
	main.cpp
	controller.cpp
	socketipc.cpp
	ipcmessage.cpp
	${PROJECT_SOURCE_DIR}/common/config.cpp
)

TARGET_LINK_LIBRARIES(controller
	${DEPENDENCIES_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${PELAGICORE_UTILITIES_LIBRARIES}
)

install(TARGETS controller RUNTIME DESTINATION ${CONTROLLER_DIR})

add_executable(application-starter
	applicationstarter.cpp
)

TARGET_LINK_LIBRARIES(application-starter
	${PELAGICORE_UTILITIES_LIBRARIES}
)

install(TARGETS application-starter RUNTIME DESTINATION ${CONTROLLER_DIR})
