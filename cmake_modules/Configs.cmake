
# Copyright (C) 2016-2017 Pelagicore AB
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


# macro: defer_add_definitions
#
# Postpone a call to add_definitions(DEF VALUE)
#
# The call to actually set DEF to VALUE with add_definitions() will be triggered
# by a call to set_all_deferred_config_definitions() which should be done when we are sure
# there will be no more changes to the definition.
#
macro(defer_add_definitions DEF VALUE)
    list(APPEND defs ${DEF})
    add_to_defs_map(${DEF} ${VALUE})
endmacro()


# macro: add_to_defs_map
#
# Adds VALUE to variable defs_map_<KEY>
#
# E.g. a call like: add_to_defs_map("MyKey" "MyValue") will make "MyValue"
# accessible through the variable "defs_map_MyKey".
#
macro(add_to_defs_map KEY VALUE)
    set("defs_map_${KEY}" ${VALUE})
endmacro()


# macro: set_all_deferred_config_definitions
#
# Call add_definitions() for all definitions previously deferred
#
macro(set_all_deferred_config_definitions)
    list(LENGTH defs count)
    math(EXPR count "${count}-1")
    foreach(i RANGE ${count})
        list(GET defs ${i} DEFINITION_KEY)
        add_definitions(${DEFINITION_KEY}="${defs_map_${DEFINITION_KEY}}")
    endforeach()
endmacro()


# macro: add_string_config
#
# GROUP - the name of the config group
# KEY - the name of the config key
# DEFAULT_VALUE - the value to be used for this config in the "default config source"
# CONFIG_FILE_VALUE - the value to set in the config file, usually this shouild be the same
#                     as the default.
#
# After a call to this macro the following variables are available:
#   * <GROUP>_<KEY> - set to <DEFAULT_VALUE>
#   * <GROUP>_<KEY>_CONFIG_FILE_VAR - set to <CONFIG_FILE_VALUE>
#
macro(add_string_config GROUP KEY DEFAULT_VALUE CONFIG_FILE_VALUE)
    set(UNIQUE_KEY "${GROUP}_${KEY}")
    set(DEFAULT_VALUE_DEFINITION "-D${UNIQUE_KEY}")
    add_definitions(${DEFAULT_VALUE_DEFINITION}="${DEFAULT_VALUE}")

    set(MANDATORY_DEFINITION "-D${UNIQUE_KEY}_MANDATORY_FLAG")

    # We can't add definitions now since they might be redefined later, so defer the actual
    # call to 'add_definitions()' to later when we know there will be no more changes to
    # definitons. A call to 'set_all_deferred_config_definitions()' will trigger the deferred adding
    # of definitions.
    defer_add_definitions(${MANDATORY_DEFINITION} FALSE)

    # Set the key so it's available for other exports and uses of the value in the cmake files.
    set(${UNIQUE_KEY} "${DEFAULT_VALUE}")

    set("${UNIQUE_KEY}_ACTIVATE" "# ")
    set("${UNIQUE_KEY}_CONFIG_FILE_VAR" "${CONFIG_FILE_VALUE}")
endmacro()


# Same as above but without the quoted values (which are not needed for non string values).
macro(add_integer_config GROUP KEY DEFAULT_VALUE CONFIG_FILE_VALUE)
    set(UNIQUE_KEY "${GROUP}_${KEY}")
    set(DEFAULT_VALUE_DEFINITION "-D${UNIQUE_KEY}")
    add_definitions(${DEFAULT_VALUE_DEFINITION}=${DEFAULT_VALUE})

    set(MANDATORY_DEFINITION "-D${UNIQUE_KEY}_MANDATORY_FLAG")

    # We can't add definitions now since they might be redefined later, so defer the actual
    # call to 'add_definitions()' to later when we know there will be no more changes to
    # definitons. A call to 'set_all_deferred_config_definitions()' will trigger the deferred adding
    # of definitions.
    defer_add_definitions(${MANDATORY_DEFINITION} FALSE)

    # Set the key so it's available for other exports and uses of the value in the cmake files.
    set(${UNIQUE_KEY} ${DEFAULT_VALUE})

    set("${UNIQUE_KEY}_ACTIVATE" "# ")
    set("${UNIQUE_KEY}_CONFIG_FILE_VAR" ${CONFIG_FILE_VALUE})
endmacro()


# The CMake "types" TRUE and FALSE should be used for DEFAULT_VALUE and strings
# should be used for CONFIG_FILE_VALUE, i.e. "true" and "false".
#
# The boolean and integer configs are added in the same way, so this macro just
# passes along the args to add_boolean_config().
#
macro(add_boolean_config GROUP KEY DEFAULT_VALUE CONFIG_FILE_VALUE)
    add_integer_config(${GROUP} ${KEY} ${DEFAULT_VALUE} ${CONFIG_FILE_VALUE})
endmacro()


# Makes the config specified by GROUP and KEY mandatory by making it un-commented
# in the    main config file, and setting a definition that will be checked by the
# config code in SC.
macro(make_config_mandatory GROUP KEY)
    set(UNIQUE_KEY "${GROUP}_${KEY}")
    set("${UNIQUE_KEY}_ACTIVATE" "")
    set(DEFINITION "-D${UNIQUE_KEY}_MANDATORY_FLAG")
    defer_add_definitions(${DEFINITION} TRUE)
endmacro()


# Un-comment a previously added config, specified by GROUP and KEY, and set it to VALUE.
# This does not make it mandatory.
macro(enable_config_in_file GROUP KEY VALUE)
    set(UNIQUE_KEY "${GROUP}_${KEY}")
    set("${UNIQUE_KEY}_ACTIVATE" "")
    set("${UNIQUE_KEY}_CONFIG_FILE_VAR" ${VALUE})
endmacro()
