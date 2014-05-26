
link_directories(
	${DEPENDENCIES_LIBRARY_DIRS})

add_executable(controller
	main.cpp
	controller.cpp
	fifoipc.cpp
	ipcmessage.cpp
)

TARGET_LINK_LIBRARIES(controller
	${DEPENDENCIES_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${PELAGICORE_UTILITIES_LIBRARIES}
)

install(TARGETS controller RUNTIME DESTINATION bin)
