#!/bin/bash

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

set -e

### Abort if the build is dirty
if [[ "`git diff --shortstat 2> /dev/null | tail -n1`" != "" ]]
then
    exit 1
fi

### Fetch the gh-pages branch and switch to it ###
git fetch origin gh-pages
git checkout -qf FETCH_HEAD
git checkout -f gh-pages || git checkout -bf gh-pages
git reset --hard FETCH_HEAD

### Setting git author ###
if [[ "`git config user.name`" == "" ]]
then
    git config user.email "noreply@pelagicore.com"
    git config user.name "Jenkins"
fi

### Moving docs to root directory and committing ###
INDEX_PAGE="index.html"
API_DOCS="api-docs"
USER_DOCS="user-docs"

git rm "${INDEX_PAGE}"    || true
git rm -rf "${API_DOCS}"  || true
git rm -rf "${USER_DOCS}" || true
cp    build/doc/index.html "${INDEX_PAGE}"
cp -r build/doc/doxygen-docs/html "${API_DOCS}"
cp -r build/doc/html "${USER_DOCS}"

git add "${INDEX_PAGE}" "${API_DOCS}" "${USER_DOCS}"
git commit -m "Built new docs on master"

### Pushing to github ###
git push origin gh-pages -f
