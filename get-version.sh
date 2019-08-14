#!/bin/sh
# print the human-readable version string in version.h

cd "`dirname $0`"
sed -n 's/#define VERSION_STRING "\([^"]*\)"/\1/p' version.h
