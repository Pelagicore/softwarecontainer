#!/usr/bin/env python

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


import os
import argparse

import sys

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')

    parser.add_argument('mode', choices=["create", "check", "delete"],
                        help='Mode of program, create, check and delete supported')

    parser.add_argument('name', type=str, default='/lala.txt',
                        help='Filename to be used')

    args = parser.parse_args()

    if args.mode == 'create':
        f = open(args.name, 'w')
        f.write('eriojweroijerfioerjf')
        f.close()
        exit(0)
    elif args.mode == 'check':
        retval = os.path.exists(args.name)
        exit(retval)
    elif args.mode == 'delete':
        if os.path.exists(args.name):
            os.remove(args.name)
            exit(1)
        exit(0)