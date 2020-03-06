TEMPLATE += app
QT += widgets
HEADERS       = includes/cpbase.h \
                includes/alsabackend.h \
                includes/convdialog.h \
                includes/cpplug.h \
                includes/cpupmix.h \
                includes/cpvdownmix.h \
                includes/generatedialog.h \
                includes/generic.h \
                includes/hwdialog.h \
                includes/ladspadialog.h \
                includes/meterdialog.h \
                includes/ratedialog.h \
                includes/renderarea.h \
                includes/mainwindow.h \
                includes/cphw.h \
                includes/cpinp.h \
                includes/cpnull.h \
                includes/cpfile.h \
                includes/cprate.h \
                includes/cproute.h \
                includes/cpdmix.h \
                includes/cpmeter.h \
                includes/cpconv.h \
                includes/cpladspa.h \
                includes/routedialog.h \
                includes/sampleplayer.h
SOURCES       = main.cpp \
                alsabackend.cpp \
                components/cpplug.cpp \
                components/cpupmix.cpp \
                components/cpvdownmix.cpp \
                dialogs/convdialog.cpp \
                dialogs/generatedialog.cpp \
                dialogs/hwdialog.cpp \
                dialogs/ladspadialog.cpp \
                dialogs/meterdialog.cpp \
                dialogs/ratedialog.cpp \
                dialogs/routedialog.cpp \
                dialogs/sampleplayer.cpp \
                renderarea.cpp \
                mainwindow.cpp \
                components/cpbase.cpp \
                components/cphw.cpp \
                components/cpinp.cpp \
                components/cpnull.cpp \
                components/cpfile.cpp \
                components/cprate.cpp \
                components/cproute.cpp \
                components/cpdmix.cpp \
                components/cpmeter.cpp \
                components/cpconv.cpp \
                components/cpladspa.cpp
RESOURCES     = aplugedit.qrc
FORMS         = \
                ui/convdlg.ui \
                ui/generatedlg.ui \
                ui/hintdlg.ui \
                ui/hwdlg.ui \
                ui/ladspadlg.ui \
                ui/mainwindow.ui \
                ui/meterdlg.ui \
                ui/ratedlg.ui \
                ui/routedlg.ui \
                ui/sampleplayer.ui \
                ui/upmixdlg.ui

CONFIG += warn_on \
    link_pkgconfig \
    rtti \
    c++14

PKGCONFIG += alsa

packagesExist(gstreamer-1.0) {
    PKGCONFIG += gstreamer-1.0
    CONFIG += use_gst
    DEFINES += WITH_GST=1
    message("GStreamer support: YES")
}

!use_gst {
    message("GStreamer support: NO")
}

# warn on *any* usage of deprecated APIs
DEFINES += QT_DEPRECATED_WARNINGS

# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS aplugedit.pro images
target.path = /usr/local/bin
INSTALLS += target
