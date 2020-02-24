TEMPLATE += app
QT += widgets
HEADERS       = includes/cpbase.h \
		includes/ladspa.h \
                includes/renderarea.h \
                includes/mainwindow.h \
                includes/cphw.h \
                includes/cpinp.h \
                includes/qhwdialog.h \
                includes/cpnull.h \
                includes/cpfile.h \
                includes/cprate.h \
                includes/qratedialog.h \
                includes/qroutedialog.h \
                includes/cproute.h \
                includes/cpdmix.h \
                includes/qmeterdialog.h \
                includes/cpmeter.h \
                includes/qconvdialog.h \
                includes/cpconv.h \
                includes/qladspadialog.h \
                includes/cpladspa.h \
                includes/qgeneratedialog.h
SOURCES       = main.cpp \
                renderarea.cpp \
                mainwindow.cpp \
                components/cpbase.cpp \
                components/cphw.cpp \
                components/cpinp.cpp \
                dialogs/qhwdialog.cpp \
                components/cpnull.cpp \
                components/cpfile.cpp \
                components/cprate.cpp \
                dialogs/qratedialog.cpp \
                dialogs/qroutedialog.cpp \
                components/cproute.cpp \
                components/cpdmix.cpp \
                dialogs/qmeterdialog.cpp \
                components/cpmeter.cpp \
                dialogs/qconvdialog.cpp \
                components/cpconv.cpp \
                dialogs/qladspadialog.cpp \
                components/cpladspa.cpp \
                dialogs/qgeneratedialog.cpp
RESOURCES     = aplugedit.qrc
FORMS         = ui/qhwdlg.ui \
                ui/qratedlg.ui \
                ui/qroutedlg.ui \
                ui/qmeterdlg.ui \
                ui/qconvdlg.ui \
                ui/qladspadlg.ui \
                ui/qgeneratedlg.ui

LIBS += -lasound -ldl

CONFIG += warn_on
# install
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS aplugedit.pro images
target.path = /usr/local/bin
INSTALLS += target
