
if(${CMAKE_INSTALL_PREFIX} STREQUAL "/usr")
	SET(CONTROLLER_DIR /var/launcher/bin/)
else()
	SET(CONTROLLER_DIR ${CMAKE_INSTALL_PREFIX}/bin)
endif()

link_directories(
	${DEPENDENCIES_LIBRARY_DIRS})

add_executable(controller
	main.cpp
	controller.cpp
	socketipc.cpp
	ipcmessage.cpp
)

TARGET_LINK_LIBRARIES(controller
	${DEPENDENCIES_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${PELAGICORE_UTILITIES_LIBRARIES}
)

install(TARGETS controller RUNTIME DESTINATION ${CONTROLLER_DIR})
