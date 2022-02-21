# Getting started

In order to contribute to or build `acl-test-data` you will first need to setup your environment.

## Setting up your environment

1. Install *CMake*, *Python 3*, and the proper compiler for your platform.
2. Execute `git submodule update --init` to get the submodules (e.g. ACL).
3. Generate the IDE solution with: `python make.py`  
   The solution is generated under `./build`  
4. Build the IDE solution with: `python make.py -build`

## Commit message format

This library uses the [angular.js message format](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#commits) and it is enforced with commit linting through every pull request.
