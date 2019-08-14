qmpwidget - A Qt widget for embedding MPlayer
=============================================

This is a small class which allows Qt developers to embed an MPlayer
instance into their application for convenient video playback. Starting
with version 4.4, Qt can be build with Phonon, the KDE multimedia
framework (see http://doc.trolltech.com/phonon-overview.html for an
overview). However, the Phonon API provides only a limited amount of
functionality, which may be too limited for some purposes.

In contrast, this class provides a way of embedding a full-fledged movie
player within an application. This means you can use all of MPlayer's
features, but also that you will need to ship a MPlayer binary with your
application if you can't make sure that it is already installed on a
user system.

For more information about MPlayer, please visit http://www.mplayerhq.hu/.

QMPwidget is written by Jonas Gehring and licensed under GPLv3. For more
information about QMPwidget, please visit http://qmpwidget.sourceforge.net/.
