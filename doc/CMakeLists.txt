
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

cmake_minimum_required(VERSION 2.6)

#
# First, Sphinx for documentation that is not code
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
find_package(Sphinx REQUIRED)

if(ENABLE_USER_DOC)
    if(NOT DEFINED SPHINX_THEME)
        set(SPHINX_THEME default)
    endif()

    if(NOT DEFINED SPHINX_THEME_DIR)
        set(SPHINX_THEME_DIR)
    endif()

    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

    # PDF output directory
    set(SPHINX_PDF_DIR "${CMAKE_CURRENT_BINARY_DIR}/pdflatex")

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
        "${BINARY_BUILD_DIR}/conf.py"
        @ONLY)

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/index.html.in"
        "${CMAKE_CURRENT_BINARY_DIR}/index.html"
        @ONLY)

    add_custom_target(sphinx-html
        ${SPHINX_EXECUTABLE}
            -q -b html
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${SPHINX_HTML_DIR}"
        COMMENT "Building HTML documentation with Sphinx"
    )

    add_custom_target(sphinx-latexpdf
        ${SPHINX_EXECUTABLE}
            -q -b latex
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${SPHINX_PDF_DIR}"
        COMMAND make -C "${SPHINX_PDF_DIR}" all-pdf > /dev/null
        COMMENT "Building Latex/PDF documentation with Sphinx")

    add_custom_target(user-doc ALL)
    add_dependencies(user-doc sphinx-html)

    option(PDF_DOCS "Enable building documentation in PDF format" OFF)
    if(PDF_DOCS)
        add_dependencies(user-doc sphinx-latexpdf)
    endif()
endif()

#
# Then, doxygen for stuff that is code.
#
if(ENABLE_API_DOC)
    # Set a Doxygen tag.
    # The tagName parameter is the name of the tag to set.
    # The value parameter is the value the specified tag will be set to.
    macro(set_doxygen_tag tagName value)
        set(tagMap_${tagName} ${value})
    endmacro()

    #
    # Add a doxygen documentation to the targets to be built and installed.
    # The packageName parameter defines the name of the package.
    # This macro uses the variables created by the user calling 'set_doxygen_tag'
    # to set the corresponding variables for the Doxygen configuraton generation.
    # If no source code path is set using 'set_doxygen_tag' the 'INPUT' tag will
    # default to the project root source directory.
    #
    macro(add_doxygen_doc packageName)
        find_package(Doxygen)

        if (DOXYGEN_FOUND STREQUAL "NO")
            message(FATAL_ERROR "Doxygen not found. Please get a copy http://www.doxygen.org")
        else()
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

        endif()
    endmacro()

    # If we want to build docs separately then this won't be set
    if(DEFINED ${PROJECT_SOURCE_DIR})
        set(MAIN_DIR ${PROJECT_SOURCE_DIR})
    else()
        set(MAIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
    endif()

    set_doxygen_tag("INPUT" "${MAIN_DIR}/doc \
                             ${MAIN_DIR}/agent \
                             ${MAIN_DIR}/common \
                             ${MAIN_DIR}/examples \
                             ${MAIN_DIR}/libsoftwarecontainer"
    )

    set_doxygen_tag("EXCLUDE_PATTERNS" "*/unit-test/* */component-test/* ")

    add_doxygen_doc(softwarecontainer)

    add_custom_target(api-doc ALL)
    add_dependencies(api-doc doxygen)
endif()
