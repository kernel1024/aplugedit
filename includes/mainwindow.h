/***************************************************************************
*   Copyright (C) 2006 - 2020 by kernelonline@gmail.com                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H 1

#include <functional>
#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include "ui_mainwindow.h"
#include "renderarea.h"
#include "sampleplayer.h"
#include "mixerwindow.h"

class ZMainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
private:
    bool m_modified;
    bool m_startMinimized;
    QString m_workFile;
    QString m_programTitle;
    QTimer m_repaintTimer;
    QString m_argumentsHelpText;

#ifdef WITH_GST
    QScopedPointer<ZSamplePlayer,QScopedPointerDeleteLater> samplePlayer;
#endif
    QScopedPointer<ZMixerWindow,QScopedPointerDeleteLater> mixerWindow;
    QScopedPointer<QSystemTrayIcon,QScopedPointerDeleteLater> trayIcon;
    QScopedPointer<QLocalServer,QScopedPointerDeleteLater> ipcServer;

    ZRenderArea *renderArea;
    QLabel* statusLabel;
    QLabel* mouseLabel;

    void clearSchematic(const std::function<void()> &callback);
    bool windowCloseRequested();
    QScreen *getCurrentScreen();
    bool setupIPC();
    void sendIPCMessage(QLocalSocket *socket, const QString &msg);
    void ipcMessageReceived();

public:
    explicit ZMainWindow(QWidget *parent = nullptr);
    ~ZMainWindow() override;
    
    bool isStartMinimized() const;

public Q_SLOTS:
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void fileGenerate();
    void fileGeneratePart();
    void editComponent();
    void toolAllocate();
    void toolSamplePlayer();
    void toolMixer();
    void helpAbout();
    void helpArguments();
    void repaintWithConnections();
    void changingComponents(ZCPBase *base);
    void updateStatus();
    void loadFile(const QString& fname);
    bool saveFile(const QString& fname);
    void systemTrayClicked(QSystemTrayIcon::ActivationReason reason);
    void restoreMainWindow();
    void closeApp();

Q_SIGNALS:
    void alsaConfigUpdated();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};
#endif
