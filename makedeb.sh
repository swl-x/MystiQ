#!/bin/bash

# Basic vars

TMPDIR=`mktemp -d`
VERSION=`./get-version.sh`
PWD=`pwd`
NAME="mystiq"
SRCDIR="$TMPDIR/$NAME-$VERSION"

# dhmake and pkg vars
MAINT="Pavel Milanes Costa"
EMAIL="pavelmc@gmail.com"
LIC="gpl3"

mkdir "$SRCDIR"
cp -r "$PWD"/* "$SRCDIR"/
cd "$SRCDIR"

# cleanup
find $PWD \( -name "moc_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
qmake .
make clean
rm -rdf AppDir
rm -rdf packaging-Rosa-rpm
rm *.deb

# make it happen
dh_make  -c $LIC -e $EMAIL -s -p $NAME -y --createorig

# make the debian archive
debuild -us -uc

cd ..
cp *.deb "$HOME"/

rm -rdf "$TMPDIR"
