#-------------------------------------------------
#
# Project created by Maikel Llamaret Heredia 2019-08-08T12:27:54
#
#-------------------------------------------------

QT       += core gui network opengl multimedia qml widgets quick quickwidgets
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
    services/ffplaypreviewer.cpp \
    services/abstractpreviewer.cpp \
    services/mplayerpreviewer.cpp \
    ui/previewdialog.cpp \
    ui/interactivecuttingdialog.cpp \
    ui/myqmpwidget.cpp \
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
    services/ffplaypreviewer.h \
    services/abstractpreviewer.h \
    services/mplayerpreviewer.h \
    ui/previewdialog.h \
    ui/interactivecuttingdialog.h \
    ui/mediaplayerwidget.h \
    ui/myqmpwidget.h \
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
    translations/mystiq_fr.ts \
    translations/mystiq_de.ts

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
    desktop.path = /usr/share/applications/
    desktop.files += mystiq.desktop
    desktop.uninstall += rm ${INSTALL_ROOT}/usr/share/icons/mystiq.png
    desktop.extra += mkdir -p ${INSTALL_ROOT}/usr/share/icons && \
                     cp icons/mystiq_96x96.png ${INSTALL_ROOT}/usr/share/icons/mystiq.png
    INSTALLS += target desktop

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
    RC_FILE = appicon_os2.rc
    # Shutdown not yet implemented on OS/2 Warp
    # When it is done, uncomment following lines and do proper modifications
    #LIBS +=
    #SOURCES -= services/powermanagement-dummy.cpp
    #SOURCES += services/powermanagement-os2.cpp
}

# This string is shown in the about box.
DEFINES += VERSION_ID_STRING=$(VERSION_ID_STRING)

# External Short Blocking Operation Timeout
DEFINES += OPERATION_TIMEOUT=30000
DEFINES += DEFAULT_THREAD_COUNT=1

# QMPwidget (embedded mplayer)
SOURCES += qmpwidget/qmpwidget.cpp
HEADERS += qmpwidget/qmpwidget.h
INCLUDEPATH += qmpwidget
#CONFIG += qmpwidget_pipemode
!win32:qmpwidget_pipemode: {
    HEADERS += qmpwidget/qmpyuvreader.h
    DEFINES += QMP_USE_YUVPIPE
}

OTHER_FILES +=

DISTFILES +=
