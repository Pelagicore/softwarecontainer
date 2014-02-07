
link_directories(
	${DEPENDENCIES_LIBRARY_DIRS} ${PELAGICORE_UTILS_LIBRARY_DIRS})

add_executable(controller
	main.cpp
	controller.cpp
)

TARGET_LINK_LIBRARIES(controller
	${PELAGICORE_UTILS_LIBRARIES}
	${DEPENDENCIES_LIBRARIES}
)

install(TARGETS controller RUNTIME DESTINATION bin)
