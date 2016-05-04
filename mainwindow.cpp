/***************************************************************************
*   Copyright (C) 2006 by Kernel                                          *
*   kernelonline@bk.ru                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
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

#include <stdlib.h>
#include <time.h>
#include "includes/mainwindow.h"
#include "includes/qgeneratedialog.h"
#include "includes/cpbase.h"
#include "includes/cphw.h"
#include "includes/cpinp.h"
#include "includes/cpnull.h"
#include "includes/cpfile.h"
#include "includes/cprate.h"
#include "includes/cproute.h"
#include "includes/cpdmix.h"
#include "includes/cpmeter.h"
#include "includes/cpconv.h"
#include "includes/cpladspa.h"

QAPEWindow::QAPEWindow()
{
  resize(QSize(800, 500).expandedTo(minimumSizeHint()));
  
  actFileNew=new QAction(QIcon(":/images/filenew.png"), tr("&New"), this);
  actFileNew->setStatusTip(tr("Create new file"));
  actFileOpen=new QAction(QIcon(":/images/fileopen.png"), tr("&Open..."), this);
  actFileOpen->setShortcut(tr("Ctrl+O"));
  actFileOpen->setStatusTip(tr("Open an existing file"));
  actFileSave=new QAction(QIcon(":/images/filesave.png"), tr("&Save"), this);
  actFileSave->setShortcut(tr("Ctrl+S"));
  actFileSave->setStatusTip(tr("Save file"));
  actFileSaveAs=new QAction(QIcon(":/images/filesaveas.png"), tr("Save &as..."), this);
  actFileSaveAs->setStatusTip(tr("Save file with another name"));
  actFileGenerate=new QAction(QIcon(":/images/filegenerate.png"), tr("&Generate ~/.asoundrc directly"), this);
  actFileGenerate->setStatusTip(tr("Generate config file for ALSA from current file  to ~/.asoundrc directly"));
  actFileGeneratePart=new QAction(QIcon(":/images/filegeneratepart.png"), tr("&Generate config..."), this);
  actFileGeneratePart->setStatusTip(tr("Generate config file for ALSA from current file"));
  actFileExit=new QAction(QIcon(":/images/fileexit.png"), tr("E&xit"), this);
  actFileExit->setStatusTip(tr("Close program"));
  
  actEditHW=new QAction(QIcon(":/images/hw.png"), tr("&HW output"), this);
  actEditHW->setStatusTip(tr("Add HW output block"));
  actEditInp=new QAction(QIcon(":/images/inp.png"), tr("DSP &input"), this);
  actEditInp->setStatusTip(tr("Add DSP input block"));
  actEditNull=new QAction(QIcon(":/images/null.png"), tr("Dummy &null output"), this);
  actEditNull->setStatusTip(tr("Add dummy null output block"));
  actEditFile=new QAction(QIcon(":/images/file.png"), tr("Write to &file"), this);
  actEditFile->setStatusTip(tr("Add block, that writes stream to file"));
  
  actEditRoute=new QAction(QIcon(":/images/route.png"), tr("&Route plugin"), this);
  actEditRoute->setStatusTip(tr("Add route block for routing and duplicating channels"));
  actEditRate=new QAction(QIcon(":/images/rate.png"), tr("R&ate plugin"), this);
  actEditRate->setStatusTip(tr("Add samplerate conversion block"));
  actEditDmix=new QAction(QIcon(":/images/dmix.png"), tr("D&mix pugin"), this);
  actEditDmix->setStatusTip(tr("Add dmix mixing and samplerate conversion block"));
  actEditLADSPA=new QAction(QIcon(":/images/ladspa.png"), tr("&LADSPA plugin thunk"), this);
  actEditLADSPA->setStatusTip(tr("Add some LADSPA effect block"));
  actEditMeter=new QAction(QIcon(":/images/meter.png"), tr("Me&ter plugin"), this);
  actEditMeter->setStatusTip(tr("Add volume meter plugin"));
  
  actEditLinear=new QAction(QIcon(":/images/linear.png"), tr("Li&near<->linear"), this);
  actEditLinear->setStatusTip(tr("Add linear<->linear format conversion block"));
  actEditFloat=new QAction(QIcon(":/images/float.png"), tr("Fl&oat<->linear"), this);
  actEditFloat->setStatusTip(tr("Add float<->linear format conversion block"));
  actEditIEC958=new QAction(QIcon(":/images/iec.png"), tr("IE&C-958<->linear"), this);
  actEditIEC958->setStatusTip(tr("Add IEC-958<->linear frames format conversion block"));
  actEditMuLaw=new QAction(QIcon(":/images/mulaw.png"), tr("M&u-Law<->linear"), this);
  actEditMuLaw->setStatusTip(tr("Add mu-Law<->linear format conversion block"));
  actEditALaw=new QAction(QIcon(":/images/alaw.png"), tr("A-La&w<->linear"), this);
  actEditALaw->setStatusTip(tr("Add A-Law<->linear format conversion block"));
  actEditImaADPCM=new QAction(QIcon(":/images/ima.png"), tr("IMA ADP&CM<->linear"), this);
  actEditImaADPCM->setStatusTip(tr("Add IMA ADPCM<->linear format conversion block"));
  
  actToolAllocate=new QAction(tr("&Scroll-in lost components"), this);
  actToolAllocate->setStatusTip(tr("Move lost components (accidentally moved beyond workspace) in your sight"));
  
  actHelpAbout=new QAction(tr("&About..."), this);
  actHelpAbout->setStatusTip(tr("About ALSA Plugin editor"));
  
  QToolBar *tbMain = new QToolBar(this);
  addToolBar(Qt::TopToolBarArea, tbMain);
  
  QToolBar *tbTools = new QToolBar(this);
  addToolBar(Qt::LeftToolBarArea, tbTools);

  QMenu *mFile,*mEdit,*mTools,*mHelp;
  
  mFile=menuBar()->addMenu(tr("&File"));
  mFile->addAction(actFileNew);
  mFile->addAction(actFileOpen);
  mFile->addAction(actFileSave);
  mFile->addAction(actFileSaveAs);
  mFile->addSeparator();
  mFile->addAction(actFileExit);
  
  mEdit=menuBar()->addMenu(tr("&Edit"));
  mEdit->addAction(actEditHW);
  mEdit->addAction(actEditInp);
  mEdit->addAction(actEditNull);
  mEdit->addAction(actEditFile);
  mEdit->addSeparator()->setText(tr("Routing, mixing, effects"));
  mEdit->addAction(actEditRoute);
  mEdit->addAction(actEditRate);
  mEdit->addAction(actEditDmix);
  mEdit->addAction(actEditLADSPA);
  mEdit->addAction(actEditMeter);
  mEdit->addSeparator()->setText(tr("Format conversion"));
  mEdit->addAction(actEditLinear);
  mEdit->addAction(actEditFloat);
  mEdit->addAction(actEditIEC958);
  mEdit->addAction(actEditMuLaw);
  mEdit->addAction(actEditALaw);
  mEdit->addAction(actEditImaADPCM);
  
  mTools=menuBar()->addMenu(tr("&Tools"));
  mTools->addAction(actFileGenerate);
  mTools->addAction(actFileGeneratePart);
  mTools->addSeparator();
  mTools->addAction(actToolAllocate);
  
  mHelp=menuBar()->addMenu(tr("&Help"));
  mHelp->addAction(actHelpAbout);
  
  tbMain->addAction(actFileNew);
  tbMain->addAction(actFileOpen);
  tbMain->addAction(actFileSave);
  tbMain->addSeparator();
  tbMain->addAction(actFileGenerate);
  tbMain->addAction(actFileGeneratePart);
  tbMain->addSeparator();
  tbMain->addAction(actFileExit);
  
  tbTools->addAction(actEditHW);
  tbTools->addAction(actEditInp);
  tbTools->addAction(actEditNull);
  tbTools->addAction(actEditFile);
  tbTools->addSeparator();
  tbTools->addAction(actEditRoute);
  tbTools->addAction(actEditRate);
  tbTools->addAction(actEditDmix);
  tbTools->addAction(actEditLADSPA);
  tbTools->addAction(actEditMeter);
  tbTools->addSeparator();
  tbTools->addAction(actEditLinear);
  tbTools->addAction(actEditFloat);
  tbTools->addAction(actEditIEC958);
  tbTools->addAction(actEditMuLaw);
  tbTools->addAction(actEditALaw);
  tbTools->addAction(actEditImaADPCM);

  QStatusBar* statusbar = new QStatusBar(this);
  statusbar->setObjectName("statusbar");
  setStatusBar(statusbar);
  statusLabel=new QLabel(statusbar);
  statusbar->addWidget(statusLabel,100);

  scrollArea = new QScrollArea(this);
  scrollArea->setWidgetResizable(true);

  setCentralWidget(scrollArea);

  renderArea = new QRenderArea(scrollArea,scrollArea);
  scrollArea->setWidget(renderArea);

  programTitle=tr("ALSA Plugin Editor");
  workFile="";
  
  repaintTimer=0;
  deletedTimer=0;
  
  connect(actFileNew,SIGNAL(triggered()),this,SLOT(fileNew()));
  connect(actFileOpen,SIGNAL(triggered()),this,SLOT(fileOpen()));
  connect(actFileSave,SIGNAL(triggered()),this,SLOT(fileSave()));
  connect(actFileSaveAs,SIGNAL(triggered()),this,SLOT(fileSaveAs()));
  connect(actFileGenerate,SIGNAL(triggered()),this,SLOT(fileGenerate()));
  connect(actFileGeneratePart,SIGNAL(triggered()),this,SLOT(fileGeneratePart()));
  connect(actFileExit,SIGNAL(triggered()),this,SLOT(fileExit()));
  
  connect(actEditHW,SIGNAL(triggered()),this,SLOT(editHW()));
  connect(actEditInp,SIGNAL(triggered()),this,SLOT(editInp()));
  connect(actEditNull,SIGNAL(triggered()),this,SLOT(editNull()));
  connect(actEditFile,SIGNAL(triggered()),this,SLOT(editFile()));
  connect(actEditDmix,SIGNAL(triggered()),this,SLOT(editDmix()));
  connect(actEditRoute,SIGNAL(triggered()),this,SLOT(editRoute()));
  connect(actEditRate,SIGNAL(triggered()),this,SLOT(editRate()));
  connect(actEditLADSPA,SIGNAL(triggered()),this,SLOT(editLADSPA()));
  connect(actEditMeter,SIGNAL(triggered()),this,SLOT(editMeter()));
  connect(actEditLinear,SIGNAL(triggered()),this,SLOT(editLinear()));
  connect(actEditFloat,SIGNAL(triggered()),this,SLOT(editFloat()));
  connect(actEditIEC958,SIGNAL(triggered()),this,SLOT(editIEC958()));
  connect(actEditALaw,SIGNAL(triggered()),this,SLOT(editALaw()));
  connect(actEditMuLaw,SIGNAL(triggered()),this,SLOT(editMuLaw()));
  connect(actEditImaADPCM,SIGNAL(triggered()),this,SLOT(editImaADPCM()));
  
  connect(actToolAllocate,SIGNAL(triggered()),this,SLOT(toolAllocate()));
  connect(actHelpAbout,SIGNAL(triggered()),this,SLOT(helpAbout()));
  
  modified=false;
  updateStatus();
  
  srand(clock());
}

void QAPEWindow::updateStatus()
{
  if (modified)
    statusLabel->setText(tr("Modified"));
  else
    statusLabel->setText("");
    
  QString s=workFile;
  if (workFile=="") s=tr("[unnamed]");
  s=programTitle+" - "+s;
  if (modified) s+=" *";
  
  setWindowTitle(s);
}

void QAPEWindow::deletedItem(QObject *)
{
  if (renderArea->cpComponentCount()==0) deletedTimer=startTimer(250);
}

void QAPEWindow::timerEvent(QTimerEvent * event)
{
  if (event->timerId()==repaintTimer)
  {
    killTimer(repaintTimer);
    repaintTimer=0;
    renderArea->repaintConn();
    update();
  }
  else if (event->timerId()==deletedTimer)
  {
    killTimer(deletedTimer);
    continueLoading();
  }
}

void QAPEWindow::loadFile(QString & fname)
{
  loadingFile=fname;
  if (renderArea->cpComponentCount()>0)
  {
    for (int i=0;i<renderArea->children().count();i++)
    {
      QCPBase* base;
      if (!(base=qobject_cast<QCPBase*>(renderArea->children().at(i)))) continue;
      connect(base,SIGNAL(destroyed(QObject*)),this,SLOT(deletedItem(QObject*)));
    }
    renderArea->deleteComponents();
  } else
    continueLoading();
}

bool QAPEWindow::saveFile(QString & fname)
{
  bool allOk=true;
  QFile file(fname);
  if (!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(this,"Error","Cannot create file");
    return false;
  }
  QDataStream out(&file);
  try
  {
    renderArea->storeSchematic(out);
  }
  catch (const char* p)
  {
    QMessageBox::critical(this,"Error",QString::fromAscii(p));
    allOk=false;
  }
  file.close();
  return allOk;
}

void QAPEWindow::continueLoading()
{
  QFile file(loadingFile);
  if (!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this,"Error","Cann't read file");
    return;
  }
   QDataStream ins(&file);
  try
  {
    renderArea->readSchematic(ins);
    for (int i=0;i<renderArea->children().count();i++)
    {
      QCPBase* base;
      if (!(base=qobject_cast<QCPBase*>(renderArea->children().at(i)))) continue;
      connect(base,SIGNAL(componentChanged(QCPBase*)),this,SLOT(changingComponents(QCPBase*)));
    }
  }
  catch (const char* p)
  {
    renderArea->deleteComponents();
    QMessageBox::critical(this,"Error",QString::fromAscii(p));
  }
  file.close();
  repaintTimer=startTimer(500);
}

void QAPEWindow::fileNew()
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
  
  scrollArea->ensureVisible(0,0);
  renderArea->deleteComponents();
  modified=false;
  workFile="";
  updateStatus();
  repaintTimer=startTimer(1000);
}

void QAPEWindow::fileOpen()
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
  QString s = QFileDialog::getOpenFileName(
                  this,
                  tr("Choose a file"),
                  "",
                  "ALSA Plugin editor files (*.ape)");
  if (s.isEmpty()) return;
  workFile=s;
  modified=false;
  loadFile(s);
  updateStatus();
}

void QAPEWindow::fileSave()
{
  if (!modified) return;
  if (workFile=="")
  {
    QFileDialog d(this,tr("Choose a filename to save under"),"~",
                    "ALSA Plugin editor files (*.ape)");
    d.setDefaultSuffix("ape");
    d.setDirectory(QDir::currentPath());
    d.setAcceptMode(QFileDialog::AcceptSave);
    d.setConfirmOverwrite(true);
    if (!d.exec()) return;
    if (d.selectedFiles().count()==0) return;
    workFile=d.selectedFiles()[0];
  }
  if (!saveFile(workFile))
    QMessageBox::critical(this,"Error","File can't saved");
  else
    modified=false;
  updateStatus();
}

void QAPEWindow::fileSaveAs()
{
  QFileDialog d(this,tr("Choose a filename to save under"),"~",
                  "ALSA Plugin editor files (*.ape)");
  d.setDefaultSuffix("ape");
  d.setAcceptMode(QFileDialog::AcceptSave);
  d.setConfirmOverwrite(true);
  d.setDirectory(QDir::currentPath());
  if (!d.exec()) return;
  if (d.selectedFiles().count()==0) return;
  workFile=d.selectedFiles()[0];
  modified=true;
  fileSave();
}

void QAPEWindow::fileExit()
{
  close();
}

void QAPEWindow::helpAbout()
{
  QMessageBox::about(this,"ALSA plugin editor 1.0.0","ALSA plugin editor\n\n"
                          "(c) 2006 Kernel (aka _kernel_)\n"
                          "kernelonline@bk.ru\n\n"
                          "This program is provided AS IS with NO WARRANTY OF ANY KIND,\n"
                          "INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS\n"
                          "FOR A PARTICULAR PURPOSE.\n\n"
                          "This program distributed under GPL license.\n\n"
                          "This program is free software; you can redistribute it and/or\n"
                          "modify it under the terms of the GNU General Public License as\n"
                          "published by the Free Software Foundation; either version 2 of\n"
                          "the License, or (at your option) any later version. ");
}

void QAPEWindow::generateConfigToFile(QTextStream & stream)
{
  stream.setCodec("ISO 8859-1"); // Latin-1 encoding for config
  try
  {
    renderArea->doGenerate(stream);
  }
  catch (const char* p)
  {
    QMessageBox::critical(this,tr("File generation error"),QString::fromAscii(p));
  }
}

void QAPEWindow::fileGenerate()
{
  if (QMessageBox::question(0,tr("Generate config file"),tr("Generate config file for ALSA and save to ~/.asoundrc?"),QMessageBox::Ok,QMessageBox::Cancel)==QMessageBox::Ok)
  {
    QString homedir=".";
    QStringList env = QProcess::systemEnvironment();
    int env_idx = env.indexOf(QRegExp("^HOME=.*"));
    if (env_idx!=-1)
    {
      homedir=env[env_idx];
      homedir.remove("HOME=");
    }
    homedir+="/.asoundrc";
    QFile file(homedir);
    if (!file.open(QIODevice::WriteOnly))
    {
      QMessageBox::critical(this,tr("File error"),tr("Cannot create file %1").arg(homedir));
      return;
    }
    QTextStream out(&file);
    generateConfigToFile(out);
    file.close();
  }
}

void QAPEWindow::fileGeneratePart()
{
  QGenerateDialog* d=new QGenerateDialog(0);
  QByteArray buf;
  QTextStream out(&buf);
  generateConfigToFile(out);
  d->configBrowser->setPlainText(QString(buf));
  d->exec();
}

void QAPEWindow::changingComponents(QCPBase *)
{
  modified=true;
  updateStatus();
}

void QAPEWindow::closeEvent(QCloseEvent *event)
{
  if (!modified)
  {
    event->accept();
    return;
  }
  switch (QMessageBox::question(this,tr("Exit ALSA Plugin Editor"),tr("Current file has been modified and not saved. Save?"),
    QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel))
  {
    case QMessageBox::Cancel:
      event->ignore();
      return;
    case QMessageBox::Yes:
      fileSave();
      if (modified)
      {
        event->ignore();
        return;
      }
      break;
  }
  event->accept();
}

// Components tools

void QAPEWindow::editHW()
{
  QCPHW* a= new QCPHW(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qhw"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editInp()
{
  QCPInp* a= new QCPInp(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qdsp"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editNull()
{
  QCPNull* a= new QCPNull(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qnull"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editFile()
{
  QCPFile* a= new QCPFile(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qfile"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editRoute()
{
  QCPRoute* a= new QCPRoute(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qroute"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editRate()
{
  QCPRate* a= new QCPRate(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qrate"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editDmix()
{
  QCPDMix* a= new QCPDMix(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qdmix"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editLADSPA()
{
  QCPLADSPA* a= new QCPLADSPA(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qladspa"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editMeter()
{
  QCPMeter* a= new QCPMeter(renderArea,renderArea);
  a->move(100,100);
  a->setObjectName("qmeter"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editLinear()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcLinear;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editFloat()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcFloat;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editIEC958()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcIEC958;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editMuLaw()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcMuLaw;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editALaw()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcALaw;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::editImaADPCM()
{
  QCPConv* a= new QCPConv(renderArea,renderArea);
  a->alConverter=alcImaADPCM;
  a->move(100,100);
  a->setObjectName("qconv"+QString::number(renderArea->cpComponentCount(),16));
  a->show();
  connect(a,SIGNAL(componentChanged(QCPBase *)),this,SLOT(changingComponents(QCPBase *)));
  changingComponents(a);
  update();
}

void QAPEWindow::toolAllocate()
{
  for (int i=0;i<renderArea->children().count();i++)
    if (QWidget* w=qobject_cast<QWidget*>(renderArea->children().at(i)))
    {
      if ((w->x()<0) || (w->geometry().right()>renderArea->width()))
        w->move(100,w->y());
      if ((w->y()<0) || (w->geometry().bottom()>renderArea->height()))
        w->move(w->x(),100);
    }
  renderArea->update();
  renderArea->repaintConn();
}
