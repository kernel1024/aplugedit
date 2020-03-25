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

#include "includes/generic.h"
#include "includes/mainwindow.h"
#include "includes/generatedialog.h"
#include "includes/cpbase.h"
#include "includes/cpconv.h"
#include "includes/cpshare.h"
#include "includes/sampleplayer.h"

ZMainWindow::ZMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    mouseLabel = new QLabel(this);
    statusBar()->addPermanentWidget(mouseLabel);
    statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(statusLabel);

    renderArea = new ZRenderArea(scrollArea);
    scrollArea->setWidget(renderArea);

    programTitle=tr("ALSA Plugin Editor");

    repaintTimer.setInterval(1000);
    repaintTimer.setSingleShot(true);

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

    actionEditDmix->setData(QSL("ZCPShare#Dmix"));
    actionEditDshare->setData(QSL("ZCPShare#Dshare"));
    actionEditDsnoop->setData(QSL("ZCPShare#Dsnoop"));

    actionEditLinear->setData(QSL("ZCPConv#Linear"));
    actionEditFloat->setData(QSL("ZCPConv#Float"));
    actionEditIEC958->setData(QSL("ZCPConv#IEC"));
    actionEditALaw->setData(QSL("ZCPConv#ALaw"));
    actionEditMuLaw->setData(QSL("ZCPConv#MuLaw"));
    actionEditImaADPCM->setData(QSL("ZCPConv#IMA"));

    connect(actionFileNew,&QAction::triggered,this,&ZMainWindow::fileNew);
    connect(actionFileOpen,&QAction::triggered,this,&ZMainWindow::fileOpen);
    connect(actionFileSave,&QAction::triggered,this,&ZMainWindow::fileSave);
    connect(actionFileSaveAs,&QAction::triggered,this,&ZMainWindow::fileSaveAs);
    connect(actionFileGenerate,&QAction::triggered,this,&ZMainWindow::fileGenerate);
    connect(actionFileGeneratePart,&QAction::triggered,this,&ZMainWindow::fileGeneratePart);
    connect(actionFileExit,&QAction::triggered,this,&ZMainWindow::close);

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
    connect(actionEditLinear,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditFloat,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditIEC958,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditALaw,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditMuLaw,&QAction::triggered,this,&ZMainWindow::editComponent);
    connect(actionEditImaADPCM,&QAction::triggered,this,&ZMainWindow::editComponent);

    connect(actionToolAllocate,&QAction::triggered,this,&ZMainWindow::toolAllocate);
    connect(actionToolSamplePlayer,&QAction::triggered,this,&ZMainWindow::toolSamplePlayer);

    connect(actionHelpAbout,&QAction::triggered,this,&ZMainWindow::helpAbout);
    connect(actionHelpAboutQt,&QAction::triggered,qApp,&QApplication::aboutQt);

    connect(&repaintTimer,&QTimer::timeout,this,&ZMainWindow::repaintWithConnections);

    qApp->installEventFilter(this);

#ifndef WITH_GST
    actionToolSamplePlayer->setEnabled(false);
#endif

    modified=false;
    updateStatus();

    QStringList fileNames = qApp->arguments();
    if (!fileNames.isEmpty())
        fileNames.takeFirst();
    for (const auto& arg : qAsConst(fileNames)) {
        QTimer::singleShot(500,this,[this,arg](){
            loadFile(arg);
        });
    }
}

ZMainWindow::~ZMainWindow() = default;

void ZMainWindow::updateStatus()
{
    if (modified) {
        statusLabel->setText(tr("Modified"));
    } else {
        statusLabel->setText(QString());
    }
    
    QString s = workFile;
    if (workFile.isEmpty()) s = tr("[unnamed]");
    if (modified) s.append(QSL(" *"));

    setWindowTitle(QSL("%1 - %2").arg(programTitle,s));
}

bool ZMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
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

    QFileInfo fi(file);
    bool res = false;
    if (fi.suffix().toLower()==QSL("ape")) {
        QDataStream ins(data);
        res = renderArea->readSchematicLegacy(ins);
    } else if (fi.suffix().toLower()==QSL("ape2")) {
        res = renderArea->readSchematic(data);
    }

    if (!res) {
        renderArea->deleteComponents({});
        QMessageBox::critical(this,tr("Error"),tr("Unable to open file - reading error."));
        return;
    }
    const auto cplist = renderArea->findComponents<ZCPBase*>();
    for (const auto &cp : cplist) {
        connect(cp,&ZCPBase::componentChanged,this,&ZMainWindow::changingComponents);
    }

    repaintTimer.start();
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
    return true;
}

void ZMainWindow::fileNew()
{
    if (modified)
    {
        switch (QMessageBox::question(this,tr("New file"),tr("Current file has been modified and not saved. Save?"),
                                      QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel))
        {
            case QMessageBox::Cancel:
                return;
            case QMessageBox::Yes:
                fileSave();
                if (modified) return;
                break;
        }
    }

    modified=false;
    workFile.clear();
    clearSchematic([this](){
        updateStatus();
        repaintTimer.start();
    });
}

void ZMainWindow::clearSchematic(const std::function<void()>& callback)
{
    scrollArea->ensureVisible(0,0);
    renderArea->deleteComponents(callback);
}

void ZMainWindow::fileOpen()
{
    if (modified)
    {
        switch (QMessageBox::question(this,tr("Open file"),tr("Current file has been modified and not saved. Save?"),
                                      QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel))
        {
            case QMessageBox::Cancel:
                return;
            case QMessageBox::Yes:
                fileSave();
                if (modified) return;
                break;
        }
    }
    QString s = QFileDialog::getOpenFileName(this,tr("Choose a file"),QString(),
                    tr("ALSA Plugin editor files v2 [*.ape2] (*.ape2);;ALSA Plugin editor files [*.ape] (*.ape)"));
    if (s.isEmpty()) return;

    workFile=s;
    modified=false;
    clearSchematic([this,s](){
        loadFile(s);
        updateStatus();
    });
}

void ZMainWindow::fileSave()
{
    if (!modified) return;
    if (workFile.isEmpty())
        fileSaveAs();

    if (saveFile(workFile)) {
        modified=false;
    } else {
        QMessageBox::critical(this,tr("Save file"),tr("Unable to save file"));
    }

    updateStatus();
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
    workFile=selectedFiles.first();

    if (saveFile(workFile)) {
        modified=false;
    } else {
        QMessageBox::critical(this,tr("Save file"),tr("Unable to save file"));
    }

    updateStatus();
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

    modified=true;
    updateStatus();
}

void ZMainWindow::closeEvent(QCloseEvent *event)
{
    if (!modified) {
        event->accept();
        return;
    }
    switch (QMessageBox::question(this,tr("Exit ALSA Plugin Editor"),
                                  tr("Current file has been modified and not saved. Save?"),
                                  QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel))
    {
        case QMessageBox::Cancel:
            event->ignore();
            return;
        case QMessageBox::Yes:
            fileSave();
            if (modified) {
                event->ignore();
                return;
            }
            break;
    }
    event->accept();
}

void ZMainWindow::editComponent()
{
    auto action = qobject_cast<QAction *>(sender());
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
    ZCPBase* cp = renderArea->createCpInstance(name,pos,
                                               QSL("%1%2").arg(name.toLower()).arg(renderArea->componentCount()));
    if (cp==nullptr) return;

    if (!convMode.isEmpty()) {
        auto cnv = qobject_cast<ZCPConv *>(cp);
        if (convMode==QSL("Linear")) cnv->setConverterType(ZCPConv::alcLinear);
        else if (convMode==QSL("Float")) cnv->setConverterType(ZCPConv::alcFloat);
        else if (convMode==QSL("IEC")) cnv->setConverterType(ZCPConv::alcIEC958);
        else if (convMode==QSL("ALaw")) cnv->setConverterType(ZCPConv::alcALaw);
        else if (convMode==QSL("MuLaw")) cnv->setConverterType(ZCPConv::alcMuLaw);
        else if (convMode==QSL("IMA")) cnv->setConverterType(ZCPConv::alcImaADPCM);
    }
    if (!dmixMode.isEmpty()) {
        auto dmix = qobject_cast<ZCPShare *>(cp);
        if (dmixMode==QSL("Dmix")) dmix->setPluginMode(ZCPShare::spDMix);
        else if (dmixMode==QSL("Dshare")) dmix->setPluginMode(ZCPShare::spDShare);
        else if (dmixMode==QSL("Dsnoop")) dmix->setPluginMode(ZCPShare::spDSnoop);
    }
    cp->show();
    connect(cp,&ZCPBase::componentChanged,this,&ZMainWindow::changingComponents);
    changingComponents(cp);
    update();
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
        auto dlg = new ZSamplePlayer(this,renderArea);
        samplePlayer.reset(dlg);
        connect(this,&ZMainWindow::alsaConfigUpdated,dlg,&ZSamplePlayer::updateSinkList);
    }
    samplePlayer->show();
    samplePlayer->showNormal();
#endif
}

void ZMainWindow::helpAbout()
{
    QMessageBox::about(this,tr("ALSA plugin editor 2.0.0"),
                       tr("ALSA plugin editor\n\n"
                          "(c) 2006 - 2020 kernelonline@gmail.com\n\n"
                          "This program is provided AS IS with NO WARRANTY OF ANY KIND,\n"
                          "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS\n"
                          "FOR A PARTICULAR PURPOSE.\n\n"
                          "This program distributed under GPL license.\n\n"
                          "This program is free software; you can redistribute it and/or\n"
                          "modify it under the terms of the GNU General Public License as\n"
                          "published by the Free Software Foundation; either version 3 of\n"
                          "the License, or (at your option) any later version. "));
}
