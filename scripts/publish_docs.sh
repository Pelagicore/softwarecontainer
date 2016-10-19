#!/bin/bash
set -e

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

API_DOCS="api-docs"
USER_DOCS="user-docs"

git rm -rf "${API_DOCS}"  || true
git rm -rf "${USER_DOCS}" || true
cp -r build/doc/doxygen-docs/html "${API_DOCS}"
cp -r build/doc/html "${USER_DOCS}"

git add "${API_DOCS}" "${USER_DOCS}"
git commit -m "Built new docs on master"

### Pushing to github ###
git push origin gh-pages -f
