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
HOMEPAGE="https://mystiq.swl-x.info"
VCS="https://github.com/llamaret/mystiq.git"
DEPS="libc6, ffmpeg (>=3.0.0), qt5-default (>=5.9.0), libqt5multimedia5-plugins (>=5.9.0)"
RECOM="sox, libnotify-bin, mplayer"
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
debuild -us -uc

cd ..
cp *.deb "$HOME"/
rm -rdf "$TMPDIR"
