TEMPLATE += app
QT += widgets
HEADERS       = includes/cpbase.h \
                includes/alsabackend.h \
                includes/blacklistdialog.h \
                includes/convdialog.h \
                includes/cpasym.h \
                includes/cpblacklist.h \
                includes/cpmulti.h \
                includes/cpplug.h \
                includes/cpshare.h \
                includes/cpspeex.h \
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
                includes/cpmeter.h \
                includes/cpconv.h \
                includes/cpladspa.h \
                includes/routedialog.h \
                includes/sampleplayer.h \
                includes/sampleplayer_p.h \
                includes/sharedialog.h \
                includes/speexdialog.h \
                jsedit/jsedit.h \
                jsedit/jsedit_p.h
SOURCES       = main.cpp \
                alsabackend.cpp \
                components/cpasym.cpp \
                components/cpblacklist.cpp \
                components/cpmulti.cpp \
                components/cpplug.cpp \
                components/cpshare.cpp \
                components/cpspeex.cpp \
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
                dialogs/sharedialog.cpp \
                dialogs/speexdialog.cpp \
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
                ui/sharedialog.ui \
                ui/speexdialog.ui \
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

OTHER_FILES += \
    README.md \
    aplugedit.desktop

# install
isEmpty(INSTALL_PREFIX):INSTALL_PREFIX = /usr
TARGET       = aplugedit
TARGET.files = aplugedit
TARGET.path  = $$INSTALL_PREFIX/bin
INSTALLS    += TARGET desktop icon16 icon24 icon32 icon48 icon64 icon128 icon256 icon512

desktop.files   = aplugedit.desktop
desktop.path    = $$INSTALL_PREFIX/share/applications

icon16.files    = images/appicon/16/aplugedit.png
icon16.path     = $$INSTALL_PREFIX/share/icons/hicolor/16x16/apps

icon24.files    = images/appicon/24/aplugedit.png
icon24.path     = $$INSTALL_PREFIX/share/icons/hicolor/24x24/apps

icon32.files    = images/appicon/32/aplugedit.png
icon32.path     = $$INSTALL_PREFIX/share/icons/hicolor/32x32/apps

icon48.files    = images/appicon/48/aplugedit.png
icon48.path     = $$INSTALL_PREFIX/share/icons/hicolor/48x48/apps

icon64.files    = images/appicon/64/aplugedit.png
icon64.path     = $$INSTALL_PREFIX/share/icons/hicolor/64x64/apps

icon128.files   = images/appicon/128/aplugedit.png
icon128.path    = $$INSTALL_PREFIX/share/icons/hicolor/128x128/apps

icon256.files   = images/appicon/256/aplugedit.png
icon256.path    = $$INSTALL_PREFIX/share/icons/hicolor/256x256/apps

icon512.files   = images/appicon/512/aplugedit.png
icon512.path    = $$INSTALL_PREFIX/share/icons/hicolor/512x512/apps
