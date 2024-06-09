#-------------------------------------------------
#
# Project created by Maikel Llamaret Heredia 2019-08-08T12:27:54
#
#-------------------------------------------------

QT       += core gui network opengl multimedia qml widgets quick quickwidgets multimediawidgets
DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00
#DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CFLAGS_RELEASE += -flto
QMAKE_CXXFLAGS_RELEASE += -flto
QMAKE_LFLAGS_RELEASE += -flto
CONFIG += ltcg

TARGET = mystiq
TEMPLATE = app

SOURCES += main.cpp \
    ui/helpmystiqdialog.cpp \
    ui/progressbar.cpp \
    ui/mainwindow.cpp \
    ui/convertlist.cpp \
    ui/conversionparameterdialog.cpp \
    ui/addtaskwizard.cpp \
    ui/aboutffmpegdialog.cpp \
    ui/optionsdialog.cpp \
    ui/aboutdialog.cpp \
    ui/poweroffdialog.cpp \
    ui/rangeselector.cpp \
    ui/timerangeedit.cpp \
    converter/presets.cpp \
    converter/mediaprobe.cpp \
    converter/mediaconverter.cpp \
    converter/ffmpeginterface.cpp \
    converter/converterinterface.cpp \
    converter/conversionparameters.cpp \
    services/paths.cpp \
    services/extensions.cpp \
    services/filepathoperations.cpp \
    services/notification.cpp \
    services/notificationservice.cpp \
    services/notificationservice-qt.cpp \
    services/notificationservice-notifysend.cpp \
    services/powermanagement-dummy.cpp \
    converter/audiofilter.cpp \
    converter/exepath.cpp \
    services/versioncompare.cpp \
    services/updatechecker.cpp \
    services/httpdownloader.cpp \
    services/updateinfoparser.cpp \
    services/constants.cpp \
    services/xmllookuptable.cpp \
    ui/updatedialog.cpp \
    services/settingtimer.cpp \
    services/abstractpreviewer.cpp \
    ui/previewdialog.cpp \
    ui/interactivecuttingdialog.cpp \
    ui/mediaplayerwidget.cpp \
    ui/rangewidgetbinder.cpp

HEADERS  += \
    ui/helpmystiqdialog.h \
    ui/progressbar.h \
    ui/mainwindow.h \
    ui/convertlist.h \
    ui/conversionparameterdialog.h \
    ui/addtaskwizard.h \
    ui/aboutffmpegdialog.h \
    ui/optionsdialog.h \
    ui/aboutdialog.h \
    ui/poweroffdialog.h \
    ui/rangeselector.h \
    ui/timerangeedit.h \
    converter/presets.h \
    converter/mediaprobe.h \
    converter/mediaconverter.h \
    converter/ffmpeginterface.h \
    converter/converterinterface.h \
    converter/conversionparameters.h \
    services/paths.h \
    services/extensions.h \
    services/filepathoperations.h \
    version.h \
    services/notification.h \
    services/notificationservice.h \
    services/notificationservice-qt.h \
    services/notificationservice-notifysend.h \
    services/powermanagement.h \
    converter/audiofilter.h \
    converter/exepath.h \
    extra-translations.h \
    services/versioncompare.h \
    services/updatechecker.h \
    services/httpdownloader.h \
    services/updateinfoparser.h \
    services/constants.h \
    services/xmllookuptable.h \
    ui/updatedialog.h \
    services/settingtimer.h \
    services/abstractpreviewer.h \
    ui/previewdialog.h \
    ui/interactivecuttingdialog.h \
    ui/mediaplayerwidget.h \
    ui/rangewidgetbinder.h

FORMS    += \
    ui/conversionparameterdialog.ui \
    ui/addtaskwizard.ui \
    ui/helpmystiqdialog.ui \
    ui/mainwindow.ui \
    ui/aboutffmpegdialog.ui \
    ui/optionsdialog.ui \
    ui/aboutdialog.ui \
    ui/poweroffdialog.ui \
    ui/updatedialog.ui \
    ui/previewdialog.ui \
    ui/interactivecuttingdialog.ui \
    ui/mediaplayerwidget.ui

RESOURCES += \
    images.qrc \
    qml.qrc \
    translator.qrc

TRANSLATIONS += \
    translations/mystiq_es.ts \
    translations/mystiq_ar.ts \
    translations/mystiq_cs_CZ.ts \
    translations/mystiq_hu.ts \
    translations/mystiq_fr.ts \
    translations/mystiq_gl.ts \
    translations/mystiq_de.ts \
    translations/mystiq_id.ts \
    translations/mystiq_it.ts \
    translations/mystiq_pt.ts \
    translations/mystiq_ja.ts \
    translations/mystiq_ru.ts \
    translations/mystiq_ru_RU.ts \
    translations/mystiq_ro.ts \
    translations/mystiq_pl.ts \
    translations/mystiq_tr.ts \
    translations/mystiq_sv.ts \
    translations/mystiq_vi.ts \
    translations/mystiq_zh_CN.ts \
    translations/mystiq_new.ts

INCLUDEPATH += .

unix {
    # If DATA_PATH is set, MystiQ searches data in DATA_PATH
    # Otherwise, it uses the executable path as data path.
    DEFINES += DATA_PATH=$(DATA_PATH)
    libnotify {
        # Linux desktop notification
        HEADERS += \
            services/notificationservice-libnotify.h
        SOURCES += \
            services/notificationservice-libnotify.cpp
        # pkgconfig
        CONFIG += link_pkgconfig
        PKGCONFIG = libnotify
        DEFINES += USE_LIBNOTIFY
    }
    # Shutdown
    QT += dbus
    SOURCES -= services/powermanagement-dummy.cpp
    SOURCES += services/powermanagement-linux.cpp
    # Install
    target.path = /usr/bin/
    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files += icons/mystiq.svg
    
    man.path = /usr/share/man/man1
    man.files += man/mystiq.1.gz
    
    appdata.path = /usr/share/metainfo
    appdata.files += metainfo/mystiq.appdata.xml
    
    desktop.path = /usr/share/applications/
    desktop.files += mystiq.desktop
    
    INSTALLS += target icon desktop man appdata
}

win32 {
    # If TOOLS_IN_DATA_PATH is set, MystiQ searches for FFmpeg executables
    # in <datapath>/tools/ instead of PATH.
    DEFINES += TOOLS_IN_DATA_PATH
    # Application Icon
    RC_FILE = appicon.rc
    # Shutdown
    LIBS += -lpowrprof
    SOURCES -= services/powermanagement-dummy.cpp
    SOURCES += services/powermanagement-w32.cpp
}

os2 {
    # Application Icon
    DEFINES += TOOLS_IN_DATA_PATH
    RC_FILE = appicon_os2.rc
    # Shutdown not yet implemented on OS/2 Warp
    # When it is done, uncomment following lines and do proper modifications
    #LIBS +=
    SOURCES -= services/powermanagement-dummy.cpp
    SOURCES += services/powermanagement-macos.cpp
}

# This string is shown in the about box.
DEFINES += VERSION_ID_STRING=$(VERSION_ID_STRING)

# External Short Blocking Operation Timeout
DEFINES += OPERATION_TIMEOUT=30000
DEFINES += DEFAULT_THREAD_COUNT=1

OTHER_FILES +=

