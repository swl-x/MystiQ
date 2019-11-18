#!/bin/bash 

# ffmpeg bins
cp -v /usr/bin/ffmpeg AppDir/usr/local/bin

# sox
cp -v /usr/bin/sox AppDir/usr/local/bin
mkdir -pv AppDir/usr/lib/mime/packages
cp -v /usr/lib/mime/packages/sox AppDir/usr/lib/mime/packages/sox

# copy all the icons
mkdir -p AppDir/usr/share/icons/hicolor
RES=`cd icons && ls | grep mystiq_| cut -d "_" -f 2 | cut -d "." -f 1`
for r in `echo $RES | xargs` ; do
    target="AppDir/usr/share/icons/hicolor/$r/apps"
    mkdir -pv "$target"
    cp -v icons/mystiq_$r.png "$target/mystiq.png"
done