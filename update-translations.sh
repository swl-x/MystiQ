#!/bin/sh
# run lupdate on the project to update translation files

cd "`dirname $0`"
lupdate -locations none mystiq.pro
