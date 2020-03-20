TEMPLATE += app
QT += widgets
HEADERS       = includes/cpbase.h \
                includes/alsabackend.h \
                includes/blacklistdialog.h \
                includes/convdialog.h \
                includes/cpblacklist.h \
                includes/cpmulti.h \
                includes/cpplug.h \
                includes/cpupmix.h \
                includes/cpvdownmix.h \
                includes/generatedialog.h \
                includes/generic.h \
                includes/hwdialog.h \
                includes/ladspa_p.h \
                includes/ladspadialog.h \
                includes/ladspalistdialog.h \
                includes/meterdialog.h \
                includes/multidlg.h \
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
                includes/sampleplayer.h \
                includes/sampleplayer_p.h \
                jsedit/jsedit.h \
                jsedit/jsedit_p.h
SOURCES       = main.cpp \
                alsabackend.cpp \
                components/cpblacklist.cpp \
                components/cpmulti.cpp \
                components/cpplug.cpp \
                components/cpupmix.cpp \
                components/cpvdownmix.cpp \
                dialogs/blacklistdialog.cpp \
                dialogs/convdialog.cpp \
                dialogs/generatedialog.cpp \
                dialogs/hwdialog.cpp \
                dialogs/ladspadialog.cpp \
                dialogs/ladspalistdialog.cpp \
                dialogs/meterdialog.cpp \
                dialogs/multidlg.cpp \
                dialogs/ratedialog.cpp \
                dialogs/routedialog.cpp \
                dialogs/sampleplayer.cpp \
                dialogs/sampleplayer_p.cpp \
                generic.cpp \
                jsedit/jsedit.cpp \
                ladspa_p.cpp \
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
                ui/blacklistdialog.ui \
                ui/convdlg.ui \
                ui/errorshowdlg.ui \
                ui/generatedlg.ui \
                ui/hintdlg.ui \
                ui/hwdlg.ui \
                ui/ladspadlg.ui \
                ui/ladspalistdialog.ui \
                ui/mainwindow.ui \
                ui/meterdlg.ui \
                ui/multidlg.ui \
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
