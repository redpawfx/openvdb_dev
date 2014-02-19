#!/bin/bash

rm -rv BUILD;
mkdir BUILD;
cd BUILD;
setpkg houdini;
setpkg maya;
cmake Unix Makefiles ../openvdb
make;

