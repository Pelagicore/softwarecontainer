cmake_minimum_required(VERSION 2.6)

# Set controller rpath to allow easier use on dev systems
# This will make controller look for shared libraries in /controller/ in
# addition to LD_LIBRARY_PATH
set(CMAKE_INSTALL_RPATH "/controller/")

add_subdirectory(src)

if(ENABLE_TEST)
	add_subdirectory(unit-test)
endif(ENABLE_TEST)
