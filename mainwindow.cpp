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

#include <chrono>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

#include "includes/generic.h"
#include "includes/mainwindow.h"
#include "includes/generatedialog.h"
#include "includes/cpbase.h"
#include "includes/cpconv.h"
#include "includes/cpshare.h"

namespace CDefaults {
const auto IPCName = "org.kernel1024.aplugedit.ipc.main";
const auto ipcEOF = "\n###";
const int ipcTimeout = 1000;
}

using namespace std::chrono_literals;

ZMainWindow::ZMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QString fileName;
    QString errorMsg;
    QCommandLineParser parser;
    auto res = ZGenericFuncs::parseCommandLine(parser,&fileName,&errorMsg,&m_startMinimized);
    m_argumentsHelpText = QSL("<html><head/><body><pre>%1</pre></body></html>")
                          .arg(parser.helpText());
    switch (res) {
        case ZGenericFuncs::CommandLineOk: break;
        case ZGenericFuncs::CommandLineError: {
            QString text = QSL("<html><head/><body><h2>%1</h2><pre>%2</pre></body></html>")
                           .arg(errorMsg,parser.helpText());
            QTimer::singleShot(500ms,this,[this,text](){
                QMessageBox::warning(this, QGuiApplication::applicationDisplayName(),text);
            });
            break;
        }
        case ZGenericFuncs::CommandLineVersionRequested: {
            QTimer::singleShot(500ms,this,&ZMainWindow::helpAbout);
            break;
        }
        case ZGenericFuncs::CommandLineHelpRequested: {
            QTimer::singleShot(500ms,this,&ZMainWindow::helpArguments);
            break;
        }
    }

    QDir dir;
    if (!fileName.isEmpty())
        fileName = dir.absoluteFilePath(fileName);
    if (!setupIPC(fileName))
        ::exit(0);

    if (!fileName.isEmpty()) {
        m_startMinimized = false;
        QTimer::singleShot(500ms,this,[this,fileName](){
            loadFile(fileName);
        });
    }

    setupUi(this);
    m_programTitle=tr("ALSA Plugin Editor");

    QIcon appIcon;
    appIcon.addFile(QSL(":/appicon/16"),QSize(16,16));
    appIcon.addFile(QSL(":/appicon/24"),QSize(24,24));
    appIcon.addFile(QSL(":/appicon/32"),QSize(32,32));
    appIcon.addFile(QSL(":/appicon/48"),QSize(48,48));
    appIcon.addFile(QSL(":/appicon/64"),QSize(64,64));
    appIcon.addFile(QSL(":/appicon/128"),QSize(128,128));
    appIcon.addFile(QSL(":/appicon/256"),QSize(256,256));
    appIcon.addFile(QSL(":/appicon/512"),QSize(512,512));
    setWindowIcon(appIcon);

    mixerWindow.reset(new ZMixerWindow(this));

    mouseLabel = new QLabel(this);
    statusBar()->addPermanentWidget(mouseLabel);
    statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(statusLabel);

    renderArea = new ZRenderArea(scrollArea);
    scrollArea->setWidget(renderArea);

    m_repaintTimer.setInterval(1s);
    m_repaintTimer.setSingleShot(true);

    actionEditHW->setData(QSL("ZCPHW"));
    actionEditInp->setData(QSL("ZCPInp"));
    actionEditNull->setData(QSL("ZCPNull"));
    actionEditFile->setData(QSL("ZCPFile"));
    actionEditPlug->setData(QSL("ZCPPlug"));
    actionEditRoute->setData(QSL("ZCPRoute"));
    actionEditRate->setData(QSL("ZCPRate"));
    actionEditUpmix->setData(QSL("ZCPUpmix"));
    actionEditVDownmix->setData(QSL("ZCPVDownmix"));
    actionEditMulti->setData(QSL("ZCPMulti"));
    actionEditLADSPA->setData(QSL("ZCPLADSPA"));
    actionEditMeter->setData(QSL("ZCPMeter"));
    actionEditBlacklist->setData(QSL("ZCPBlacklist"));
    actionEditAsym->setData(QSL("ZCPAsym"));
    actionEditSpeex->setData(QSL("ZCPSpeex"));
    actionEditSoftvol->setData(QSL("ZCPSoftvol"));
    actionEditEqualizer->setData(QSL("ZCPEqual"));

    actionEditDmix->setData(QSL("ZCPShare#Dmix"));
    actionEditDshare->setData(QSL("ZCPShare#Dshare"));
    actionEditDsnoop->setData(QSL("ZCPShare#Dsnoop"));

    actionEditLinear->setData(QSL("ZCPConv#Linear"));
    actionEditFloat->setData(QSL("ZCPConv#Float"));
    actionEditIEC958->setData(QSL("ZCPConv#IEC"));
    actionEditALaw->setData(QSL("ZCPConv#ALaw"));
    actionEditMuLaw->setData(QSL("ZCPConv#MuLaw"));
    actionEditImaADPCM->setData(QSL("ZCPConv#IMA"));

    QSettings stg;
    stg.beginGroup(QSL("mainWindow"));
    actionToolUseTray->setChecked(stg.value(QSL("useTray"),true).toBool());
    stg.endGroup();

    connect(actionFileNew,&QAction::triggered,this,&ZMainWindow::fileNew);
    connect(actionFileOpen,&QAction::triggered,this,&ZMainWindow::fileOpen);
    connect(actionFileSave,&QAction::triggered,this,&ZMainWindow::fileSave);
    connect(actionFileSaveAs,&QAction::triggered,this,&ZMainWindow::fileSaveAs);
    connect(actionFileGenerate,&QAction::triggered,this,&ZMainWindow::fileGenerate);
    connect(actionFileGeneratePart,&QAction::triggered,this,&ZMainWindow::fileGeneratePart);
    connect(actionFileExit,&QAction::triggered,this,&ZMainWindow::closeApp);

    connect(actionEditHW,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditInp,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditNull,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditFile,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditPlug,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditDmix,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditDshare,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditDsnoop,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditRoute,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditRate,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditUpmix,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditVDownmix,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditMulti,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditLADSPA,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditMeter,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditBlacklist,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditAsym,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditSpeex,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditSoftvol,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditEqualizer,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditLinear,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditFloat,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditIEC958,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditALaw,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditMuLaw,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditImaADPCM,&QAction::triggered,this,&ZMainWindow::editComponent);

    connect(actionToolAllocate,&QAction::triggered,this,&ZMainWindow::toolAllocate);
    connect(actionToolSamplePlayer,&QAction::triggered,this,&ZMainWindow::toolSamplePlayer);
    connect(actionToolMixer,&QAction::triggered,this,&ZMainWindow::toolMixer);
    connect(actionToolUseTray,&QAction::triggered,this,&ZMainWindow::toolUseTray);

    connect(actionHelpAbout,&QAction::triggered,this,&ZMainWindow::helpAbout);
    connect(actionHelpAboutQt,&QAction::triggered,qApp,&QApplication::aboutQt); //NOLINT
    connect(actionHelpArguments,&QAction::triggered,this,&ZMainWindow::helpArguments);

    connect(&m_repaintTimer,&QTimer::timeout,this,&ZMainWindow::repaintWithConnections);
    connect(this,&ZMainWindow::alsaConfigUpdated,mixerWindow.data(),&ZMixerWindow::reloadAllCards);

    qApp->installEventFilter(this); //NOLINT

    setupTrayIcon();

#ifndef WITH_GST
    actionToolSamplePlayer->setEnabled(false);
#endif

    m_modified=false;
    updateStatus();
}

ZMainWindow::~ZMainWindow()
{
    QSettings stg;
    stg.beginGroup(QSL("mainWindow"));
    stg.setValue(QSL("useTray"),actionToolUseTray->isChecked());
    stg.endGroup();
}

void ZMainWindow::updateStatus()
{
    if (m_modified) {
        statusLabel->setText(tr("Modified"));
    } else {
        statusLabel->setText(QString());
    }
    
    QString s = m_workFile;
    if (s.isEmpty()) s = tr("[unnamed]");
    if (m_modified) s.append(QSL(" *"));

    setWindowTitle(QSL("%1 - %2").arg(m_programTitle,s));
}

bool ZMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        auto *mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent) {
            mouseLabel->setText(tr("X:%1, Y:%2").
                                arg(mouseEvent->pos().x()).
                                arg(mouseEvent->pos().y()));
        }
    }
    return QMainWindow::eventFilter(obj,event);
}

void ZMainWindow::loadFile(const QString &fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this,tr("Error"),tr("Unable to read file"));
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    m_modified = false;

    if (!(renderArea->readSchematic(data))) {
        renderArea->deleteComponents({});
        QMessageBox::critical(this,tr("Error"),tr("Unable to open file - reading error."));
        return;
    }
    const auto cplist = renderArea->findComponents<ZCPBase*>();
    for (const auto &cp : cplist) {
        connect(cp,&ZCPBase::componentChanged,this,&ZMainWindow::changingComponents);
    }

    m_workFile = fname;

    m_repaintTimer.start();
    m_componentCounter = renderArea->componentCount();

    updateStatus();
}

bool ZMainWindow::saveFile(const QString &fname)
{
    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this,tr("Error"),tr("Unable to create file"));
        return false;
    }
    file.write(renderArea->storeSchematic());
    file.close();
    m_modified = false;
    m_workFile = fname;

    updateStatus();

    return true;
}

void ZMainWindow::systemTrayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        if (isVisible()) {
            hide();
        } else if (mixerWindow->isVisible()) {
            mixerWindow->hide();
        } else {
            toolMixer();
        }
    }
}

void ZMainWindow::restoreMainWindow()
{
    showNormal();
    activateWindow();
    raise();
}

void ZMainWindow::closeApp()
{
    if (!windowCloseRequested()) return;

    qApp->quit(); //NOLINT
}

bool ZMainWindow::windowCloseRequested()
{
    if (!m_modified) {
        return true;
    }
    switch (QMessageBox::question(this,tr("Exit ALSA Plugin Editor"),
                                  tr("Current file has been modified and not saved. Save?"),
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
    {
        case QMessageBox::Yes:
            fileSave();
            if (m_modified) {
                return false;
            }
            break;
        case QMessageBox::No:
            return true;
        default:
            return false;
    }

    return true;
}

QScreen* ZMainWindow::getCurrentScreen()
{
    QScreen* res = nullptr;

    if (window() && window()->windowHandle()) {
        res = window()->windowHandle()->screen();
    } else if (!QApplication::screens().isEmpty()) {
        res = QApplication::screenAt(QCursor::pos());
    }
    if (res == nullptr)
        res = QApplication::primaryScreen();

    return res;
}

bool ZMainWindow::setupIPC(const QString &sendFilename)
{
    if (!ipcServer.isNull()) return false;

    QString serverName = QString::fromLatin1(CDefaults::IPCName);
    serverName.replace(QRegularExpression(QSL("[^\\w\\-.]")), QString());

    QScopedPointer<QLocalSocket> socket(new QLocalSocket());
    socket->connectToServer(serverName);
    if (socket->waitForConnected(CDefaults::ipcTimeout)){
        // Connected. Process is already running, send message to it
        if (ZGenericFuncs::runnedFromQtCreator()) { // This is debug run, try to close old instance
            // Send close request
            sendIPCMessage(socket.data(),QSL("debugRestart"));
            socket->flush();
            socket->close();
            QApplication::processEvents();
            QThread::sleep(3);
            QApplication::processEvents();
            // Check for closing
            socket->connectToServer(serverName);
            if (socket->waitForConnected(CDefaults::ipcTimeout)) { // connected, unable to close
                sendIPCMessage(socket.data(),QSL("showMainWindow#%1").arg(sendFilename));
                socket->flush();
                socket->close();
                return false;
            }
            // Old instance closed, start new one
            QLocalServer::removeServer(serverName);
            ipcServer.reset(new QLocalServer());
            ipcServer->listen(serverName);
            connect(ipcServer.data(), &QLocalServer::newConnection, this, &ZMainWindow::ipcMessageReceived);
        } else {
            sendIPCMessage(socket.data(),QSL("showMainWindow#%1").arg(sendFilename));
            socket->flush();
            socket->close();
            return false;
        }
    } else {
        // New process. Startup server normally, listen for new instances
        QLocalServer::removeServer(serverName);
        ipcServer.reset(new QLocalServer());
        ipcServer->listen(serverName);
        connect(ipcServer.data(), &QLocalServer::newConnection, this, &ZMainWindow::ipcMessageReceived);
    }
    if (socket->isOpen())
        socket->close();
    return true;
}

void  ZMainWindow::sendIPCMessage(QLocalSocket *socket, const QString &msg)
{
    if (socket==nullptr) return;

    QString s = QSL("%1%2").arg(msg,QString::fromLatin1(CDefaults::ipcEOF));
    socket->write(s.toUtf8());
}

void ZMainWindow::ipcMessageReceived()
{
    QLocalSocket *socket = ipcServer->nextPendingConnection();
    QByteArray bmsg;
    do {
        if (!socket->waitForReadyRead(CDefaults::ipcTimeout)) return;
        bmsg.append(socket->readAll());
    } while (!bmsg.contains(CDefaults::ipcEOF));
    socket->close();
    socket->deleteLater();

    QStringList cmd = QString::fromUtf8(bmsg).split('\n');
    if (cmd.first().startsWith(QSL("showMainWindow"))) {
        auto sl = cmd.first().split(QChar('#'),Qt::KeepEmptyParts);
        restoreMainWindow();
        auto fileName = sl.last();
        if (!fileName.isEmpty()) {
            clearSchematic([this,fileName](){
                loadFile(fileName);
            });
        }
    } else if (cmd.first().startsWith(QSL("debugRestart"))) {
        qInfo() << tr("Closing aplugedit instance (pid: %1)"
                      "after debugRestart request")
                   .arg(QApplication::applicationPid());
        qApp->quit(); //NOLINT
    }
}

void ZMainWindow::setupTrayIcon()
{
    if (!trayIcon.isNull() && !actionToolUseTray->isChecked()) {
        trayIcon.reset(nullptr);
    }
    if (QSystemTrayIcon::isSystemTrayAvailable() && actionToolUseTray->isChecked()) {
        trayIcon.reset(new QSystemTrayIcon(windowIcon(),this));
        trayIcon->setToolTip(m_programTitle);
        trayIcon->show();

        connect(trayIcon.data(),&QSystemTrayIcon::activated,this,&ZMainWindow::systemTrayClicked);

        auto *cm = new QMenu(this);
        auto *ac = cm->addAction(tr("Restore"));
        connect(ac,&QAction::triggered,this,&ZMainWindow::restoreMainWindow);
        ac = cm->addAction(tr("Show mixer"));
        connect(ac,&QAction::triggered,this,&ZMainWindow::toolMixer);
        cm->addSeparator();
        ac = cm->addAction(tr("Quit"));
        connect(ac,&QAction::triggered,this,&ZMainWindow::closeApp);

        trayIcon->setContextMenu(cm);
    }
}

void ZMainWindow::closeEvent(QCloseEvent *event)
{
    if (!trayIcon.isNull() && trayIcon->isVisible()) {
        hide();
        event->ignore();
        return;
    }

    if (windowCloseRequested()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void ZMainWindow::fileNew()
{
    if (m_modified)
    {
        switch (QMessageBox::question(this,tr("New file"),tr("Current file has been modified and not saved. Save?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
        {
            case QMessageBox::No:
                break;
            case QMessageBox::Yes:
                fileSave();
                if (m_modified) return;
                break;
            default:
                return;
        }
    }

    clearSchematic([this](){
        m_modified=false;
        m_workFile.clear();
        updateStatus();
        m_repaintTimer.start();
        m_componentCounter = 0;
    });
}

bool ZMainWindow::isStartMinimized() const
{
    return (m_startMinimized && !trayIcon.isNull() && trayIcon->isVisible());
}

void ZMainWindow::clearSchematic(const std::function<void()>& callback)
{
    scrollArea->ensureVisible(0,0);
    renderArea->deleteComponents(callback);
}

void ZMainWindow::fileOpen()
{
    if (m_modified)
    {
        switch (QMessageBox::question(this,tr("Open file"),tr("Current file has been modified and not saved. Save?"),
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel))
        {
            case QMessageBox::No:
                break;
            case QMessageBox::Yes:
                fileSave();
                if (m_modified) return;
                break;
            default:
                return;
        }
    }
    QString s = QFileDialog::getOpenFileName(this,tr("Choose a file"),QString(),
                    tr("ALSA Plugin editor files v2 [*.ape2] (*.ape2)"));
    if (s.isEmpty()) return;

    clearSchematic([this,s](){
        loadFile(s);
    });
}

void ZMainWindow::fileSave()
{
    if (!m_modified) return;
    if (m_workFile.isEmpty()) {
        fileSaveAs();
        return;
    }

    if (!saveFile(m_workFile)) {
        QMessageBox::critical(this,tr("Save file"),tr("Unable to save file"));
    }
}

void ZMainWindow::fileSaveAs()
{
    QFileDialog d(this,tr("Choose a filename to save under"),QString(),
                  tr("ALSA Plugin Editor files v2 [*.ape2] (*.ape2)"));
    d.setDefaultSuffix(QSL("ape2"));
    d.setAcceptMode(QFileDialog::AcceptSave);
    if (d.exec()==QDialog::Rejected) return;

    const auto selectedFiles = d.selectedFiles();
    if (selectedFiles.isEmpty()) return;

    if (!saveFile(selectedFiles.first())) {
        QMessageBox::critical(this,tr("Save file"),tr("Unable to save file"));
    }
}

void ZMainWindow::repaintWithConnections()
{
    renderArea->repaintConn();
    update();
}

void ZMainWindow::fileGenerate()
{
    if (QMessageBox::question(this,tr("Generate config file"),
                              tr("Generate config file for ALSA and save to ~/.asoundrc?"),
                              QMessageBox::Yes,QMessageBox::No)==QMessageBox::Yes)
    {
        QDir homedir = QDir::home();
        QString fileName = homedir.absoluteFilePath(QSL(".asoundrc"));
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this,tr("File error"),tr("Unable create file %1").arg(fileName));
            return;
        }
        QTextStream out(&file);
        QStringList warnings;
        renderArea->doGenerate(out,warnings);
        file.close();

        Q_EMIT alsaConfigUpdated();

        if (!warnings.isEmpty()) {
            ZGenericFuncs::showWarningsDialog(this,tr("ALSA config problems"),
                                                  tr("Config was generated with %1 warnings.").arg(warnings.count()),
                                                  warnings);
        }
        statusBar()->showMessage(tr("~/.asoundrc file was saved successfully."));
    }
}

void ZMainWindow::fileGeneratePart()
{
    ZGenerateDialog d(this);
    QString config;
    QTextStream out(&config);
    QStringList warnings;
    renderArea->doGenerate(out,warnings);
    d.setConfigText(config);
    d.setWarnings(warnings);
    d.exec();
}

void ZMainWindow::changingComponents(ZCPBase *base)
{
    Q_UNUSED(base)

    m_modified=true;
    updateStatus();
}

void ZMainWindow::editComponent()
{
    auto *action = qobject_cast<QAction *>(sender());
    if (action==nullptr) return;

    QString name = action->data().toString();
    if (name.isEmpty()) return;

    QString convMode;
    QString dmixMode;
    if (name.startsWith(QSL("ZCPConv"))) {
        QStringList sl = name.split(QSL("#"));
        name = sl.first();
        convMode = sl.last();
    } else if (name.startsWith(QSL("ZCPShare"))) {
        QStringList sl = name.split(QSL("#"));
        name = sl.first();
        dmixMode = sl.last();
    }

    QPoint pos(100,100);
    int dx = renderArea->componentCount()*10;
    pos += QPoint(dx,dx);
    ZCPBase* cp = renderArea->createCpInstance(name,pos,makeUniqueComponentName(name.toLower()));

    if (cp==nullptr) return;

    if (!convMode.isEmpty()) {
        auto *cnv = qobject_cast<ZCPConv *>(cp);
        if (convMode==QSL("Linear")) cnv->setConverterType(ZCPConv::alcLinear);
        else if (convMode==QSL("Float")) cnv->setConverterType(ZCPConv::alcFloat);
        else if (convMode==QSL("IEC")) cnv->setConverterType(ZCPConv::alcIEC958);
        else if (convMode==QSL("ALaw")) cnv->setConverterType(ZCPConv::alcALaw);
        else if (convMode==QSL("MuLaw")) cnv->setConverterType(ZCPConv::alcMuLaw);
        else if (convMode==QSL("IMA")) cnv->setConverterType(ZCPConv::alcImaADPCM);
    }
    if (!dmixMode.isEmpty()) {
        auto *dmix = qobject_cast<ZCPShare *>(cp);
        if (dmixMode==QSL("Dmix")) dmix->setPluginMode(ZCPShare::spDMix);
        else if (dmixMode==QSL("Dshare")) dmix->setPluginMode(ZCPShare::spDShare);
        else if (dmixMode==QSL("Dsnoop")) dmix->setPluginMode(ZCPShare::spDSnoop);
    }
    cp->show();
    connect(cp,&ZCPBase::componentChanged,this,&ZMainWindow::changingComponents);
    changingComponents(cp);
    update();
}

QString ZMainWindow::makeUniqueComponentName(const QString& base)
{
    QString name;
    do {
        name = QSL("%1%2").arg(base).arg(m_componentCounter++);
    } while (renderArea->findChild<ZCPBase *>(name) != nullptr);
    return name;
}

void ZMainWindow::toolAllocate()
{
    const auto wlist = renderArea->findComponents<QWidget*>();
    for (const auto &w : wlist) {
        if ((w->x()<0) || (w->geometry().right()>renderArea->width()))
            w->move(100,w->y());
        if ((w->y()<0) || (w->geometry().bottom()>renderArea->height()))
            w->move(w->x(),100);
    }
    renderArea->update();
    renderArea->repaintConn();
}

void ZMainWindow::toolSamplePlayer()
{
#ifdef WITH_GST
    if (!samplePlayer) {
        auto *dlg = new ZSamplePlayer(this,renderArea);
        samplePlayer.reset(dlg);
        connect(this,&ZMainWindow::alsaConfigUpdated,dlg,&ZSamplePlayer::updateSinkList);
    }
    samplePlayer->show();
    samplePlayer->showNormal();
#endif
}

void ZMainWindow::toolMixer()
{
    mixerWindow->show();
    mixerWindow->showNormal();
    if (!isVisible()) {
        if (mixerWindow->width()>0) {
            QScreen* screen = getCurrentScreen();
            QRect scr = screen->availableGeometry();
            QPoint center(scr.width() / 2 + scr.x(),
                          scr.height() / 2 + scr.y());
            QPoint cur = QCursor::pos(screen);
            QPoint pos = scr.topLeft();
            if (cur.x() < center.x() && cur.y() > center.y()) {
                pos.setY(scr.height() - mixerWindow->frameSize().height());
            } else if (cur.x() > center.x() && cur.y() < center.y()) {
                pos.setX(scr.width() - mixerWindow->frameSize().width());
            } else if (cur.x() > center.x() && cur.y() > center.y()) {
                pos.setY(scr.height() - mixerWindow->frameSize().height());
                pos.setX(scr.width() - mixerWindow->frameSize().width());
            }
            mixerWindow->move(pos);
        }

        mixerWindow->activateWindow();
        mixerWindow->raise();
    }
}

void ZMainWindow::toolUseTray()
{
    setupTrayIcon();
}

void ZMainWindow::helpAbout()
{
    QMessageBox::about(this,QGuiApplication::applicationDisplayName(),
                       tr("%1 %2\n\n"
                          "(c) 2006 - 2021 kernelonline@gmail.com\n\n"
                          "This program is provided AS IS with NO WARRANTY OF ANY KIND,\n"
                          "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS\n"
                          "FOR A PARTICULAR PURPOSE.\n\n"
                          "This program distributed under GPL license.\n\n"
                          "This program is free software; you can redistribute it and/or\n"
                          "modify it under the terms of the GNU General Public License as\n"
                          "published by the Free Software Foundation; either version 3 of\n"
                          "the License, or (at your option) any later version. ")
                       .arg(QGuiApplication::applicationDisplayName(),
                            QCoreApplication::applicationVersion()));
}

void ZMainWindow::helpArguments()
{
    QMessageBox::information(this, QGuiApplication::applicationDisplayName(),m_argumentsHelpText);
}
