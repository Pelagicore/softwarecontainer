#!/bin/bash

#
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
#

#
# This is a helper script for invoking uncrustify.
# Invoking this script will find all C++ files (*.cpp, *.h, *.c) in
# the 'agent', 'common' and 'libsoftwarecontainer' directories and format the
# files according to an uncrustify config file.
# The resulting files will be put in a directory called 'formatting_output'.
#
# The $UNCRUSTIFY_CONFIG_FILE environment variable can be used to specify which
# uncrustify config file to use.
#

## The root path for the code base
if [ "$COMPONENT_FOLDER" = "" ];
then
  COMPONENT_FOLDER=`pwd`
fi

if [ "$UNCRUSTIFY_CONFIG_FILE" = "" ];
then
  UNCRUSTIFY_CONFIG_FILE=`dirname $0`/uncrustify.cfg
fi

# Currently not used
if [ "$UNCRUSTIFY_LOGFILE" = "" ];
then
    UNCRUSTIFY_LOGFILE="uncrustify_output.log"
fi

declare -a dirs=("agent" "common" "libsoftwarecontainer")


echo "Formatting with Uncrustify configuration file : $UNCRUSTIFY_CONFIG_FILE"
echo "Running uncrustify starting at $COMPONENT_FOLDER"

## This will create a dir formatting_ouput with all the uncrustify result files
UNCRUSTIFY_COMMAND="uncrustify -c $UNCRUSTIFY_CONFIG_FILE --prefix formatting_output 'file'"

for subdirectory in "${dirs[@]}"
do
    echo "Formatting .cpp files..."
    find $COMPONENT_FOLDER/$subdirectory/ -name "*.cpp" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
    echo "Formatting .h files..."
    find $COMPONENT_FOLDER/$subdirectory/ -name "*.h" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
    echo "Formatting .c files..."
    find $COMPONENT_FOLDER/$subdirectory/ -name "*.c" | xargs -I file echo "$UNCRUSTIFY_COMMAND" | bash -s
done

exit
