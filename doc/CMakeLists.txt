find_package(PelagicoreUtilities)

set_doxygen_tag("INPUT" "${PROJECT_SOURCE_DIR}/doc ${PROJECT_SOURCE_DIR}/pelagicontain ${PROJECT_SOURCE_DIR}/controller")
set_doxygen_tag("EXCLUDE_PATTERNS" "*/unit-test/* */component-test/*")

add_doxygen_doc(pelagicontain)
