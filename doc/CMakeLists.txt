
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


#
# First, Sphinx for documentation that is not code
#

if(ENABLE_DOC)
    find_package(Sphinx REQUIRED)

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

    add_custom_target(html
        ${SPHINX_EXECUTABLE}
            -q -b html
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${SPHINX_HTML_DIR}"
        COMMENT "Building HTML documentation with Sphinx"
    )

    add_custom_target(latexpdf
        ${SPHINX_EXECUTABLE}
            -q -b latex
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${SPHINX_PDF_DIR}"
        COMMAND make -C "${SPHINX_PDF_DIR}" all-pdf > /dev/null
        COMMENT "Building Latex/PDF documentation with Sphinx")

    #
    # TODO: "doc" is taken by doxygen/pelagicore-utils.
    #
    add_custom_target(docs ALL)
    add_dependencies(docs html latexpdf)
endif(ENABLE_DOC)

#
# Then, doxygen for stuff that is code.
# The doxygen macro in Pelagicore-utils will check for ENABLE_DOC by itself
#

find_package(PelagicoreUtilities)

set_doxygen_tag("INPUT" "${PROJECT_SOURCE_DIR}/doc 
                         ${PROJECT_SOURCE_DIR}/agent
                         ${PROJECT_SOURCE_DIR}/common
                         ${PROJECT_SOURCE_DIR}/examples
                         ${PROJECT_SOURCE_DIR}/libsoftwarecontainer 
                         ${PROJECT_SOURCE_DIR}/libsoftwarecontainer-agent"
)
set_doxygen_tag("EXCLUDE_PATTERNS" "*/unit-test/* */component-test/*")
add_doxygen_doc(softwarecontainer)

