cmake_minimum_required(VERSION 2.6)

add_subdirectory(src)

if(ENABLE_TEST)
	add_subdirectory(unit-test)
endif(ENABLE_TEST)
