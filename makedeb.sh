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
HOMEPAGE="https://mystiqapp.com"
VCS="https://github.com/swl-x/mystiq.git"
DEPS="libc6, libnotify-bin, ffmpeg (>=3.0.0), qt5-default (>=5.10.0), libqt5multimedia5-plugins (>=5.10.0), libqt5qml5 (>=5.10.0), libqt5quickwidgets5 (>=5.10.0), qml-module-qtquick2 (>=5.10.0), qml-module-qtquick-dialogs (>=5.10.0), qml-module-qtmultimedia (>=5.10.0), libqt5opengl5 (>=5.10.0)"
RECOM="sox"
DATE=`date +"%a, %d %b 20%y %H:%M:%S %z"`

# var listing for dynamic substitution
VARS="VERSION NAME MAINT EMAIL HOMEPAGE VCS DEPS RECOM DATE"

mkdir "$SRCDIR"
cp -r "$PWD"/* "$SRCDIR"/
cd "$SRCDIR"

# cleanup
find $PWD \( -name "moc_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
qmake .
make clean
rm -rdf AppDir
rm -rdf packaging-Rosa-rpm
rm -f *.deb

# make it happen
dh_make  -c $LIC -e $EMAIL -s -p $NAME -y --createorig

# provision the debian folder
for v in `echo $VARS | xargs` ; do
    # get the var content
    CONTp=${!v}
    
    #escape possible / in the files
    CONT=`echo ${CONTp//\//\\\\/}`

    # note
    echo "replace $v by \"$CONT\""

    find "debian/" -type f -exec \
        sed -i s/"\_$v\_"/"${CONT}"/g {} \;
done

# make the debian archive
dpkg-buildpackage -us -uc
debuild -us -uc -b

cd ..
pwd
ls -la *.deb
cp mystiq*.* "$CI_PROJECT_DIR"/
cd "$CI_PROJECT_DIR"
ls -la "$CI_PROJECT_DIR"/mystiq*.*
pwd
rm -rdf "$TMPDIR"
