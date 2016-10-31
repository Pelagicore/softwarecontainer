# Copyright (C) 2016 Pelagicore AB
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
# BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
# SOFTWARE.
#
# For further information see LICENSE

find_package(Doxygen REQUIRED)

set(packageName "softwarecontainer")

# Set a Doxygen tag.
# The tagName parameter is the name of the tag to set.
# The value parameter is the value the specified tag will be set to.
macro(set_doxygen_tag tagName value)
    set(tagMap_${tagName} ${value})
endmacro()

set_doxygen_tag("INPUT" "${MAIN_DIR}/doc \
                         ${MAIN_DIR}/agent \
                         ${MAIN_DIR}/common \
                         ${MAIN_DIR}/examples \
                         ${MAIN_DIR}/libsoftwarecontainer"
)    

set_doxygen_tag("EXCLUDE_PATTERNS" "*/unit-test/* */component-test/* ")

if(DEFINED tagMap_INPUT)
    set(DOXYGEN_INPUT ${tagMap_INPUT})
else()
    set(DOXYGEN_INPUT ${PROJECT_SOURCE_DIR})
endif()

if(DEFINED tagMap_EXCLUDE_PATTERNS)
    set(DOXYGEN_EXCLUDE_PATTERNS ${tagMap_EXCLUDE_PATTERNS})
else()
    set(DOXYGEN_EXCLUDE_PATTERNS "")
endif()

if(DEFINED tagMap_EXAMPLE_PATH)
    set(DOXYGEN_EXAMPLE_PATH ${tagMap_EXAMPLE_PATH})
else()
    set(DOXYGEN_EXAMPLE_PATH "")
endif()

# prepare doxygen configuration file
set(OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
set(DOXYGEN_PROJECT_NAME ${packageName})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doxygen.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg)

# Where to place the generated doxygen documentation
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen-docs)

# add doxygen as target
add_custom_command(
    OUTPUT ${DOXYGEN_OUTPUT_DIR}
    COMMAND ${DOXYGEN_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/doxygen.cfg
    COMMENT "Building doxygen documentation"
)

# cleanup $build/doc/doxygen on "make clean"
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doxygen)

add_custom_target(
    doxygen
    DEPENDS ${DOXYGEN_OUTPUT_DIR}
)

# install HTML API documentation and manual pages
set(DOC_PATH "share/doc/${packageName}")

install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}
        DESTINATION ${DOC_PATH}
)

# If we want to build docs separately then this won't be set
if(DEFINED ${PROJECT_SOURCE_DIR})
    set(MAIN_DIR ${PROJECT_SOURCE_DIR})
else()
    set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
endif()

add_custom_target(api-doc ALL)
add_dependencies(api-doc doxygen)
add_dependencies(doc api-doc)
