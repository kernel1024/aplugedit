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

#include <stdio.h>

#include "includes/renderarea.h"
#include "includes/cpbase.h"
    
QCPBase::QCPBase(QWidget *parent, QRenderArea *aOwner)
      : QWidget(parent)
{
  pinColor=Qt::blue;
  isDragging=false;
  cpOwner=aOwner;
  fSettingsDlg=false;
}

bool QCPBase::canConnectOut(QCPBase *)
{
  return true;
}

bool QCPBase::canConnectIn(QCPBase *)
{
  return true;
}

void QCPBase::mouseInPin(const QPoint & mx, int &aPinNum, int &aPinType, QCPBase* & aFilter)
// Note: x and y must be in global screen coordinate system.
{
  QRect r;
  QPoint p;
  aPinNum=-1;
  aPinType=QPT_INPUT;
  aFilter=0;
  for (int i=0; i<cpOwner->children().count(); i++)
  {
    QObject* oc=cpOwner->children().at(i);
    QCPBase *c=qobject_cast<QCPBase*>(oc);
    if (c==0) continue;
    for (int j=0; j<c->fInputs.count(); j++)
    {
      QCPInput* a=c->fInputs[j];
      p=QPoint(cpOwner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+QCP_PINSIZE/2,a->ownerFilter->y()+a->relCoord.y()+QCP_PINSIZE/2)));
      r=QRect(p.x()-QCP_PINSIZE,p.y()-QCP_PINSIZE,2*QCP_PINSIZE,2*QCP_PINSIZE);
      if (r.contains(mx.x(),mx.y()))
      {
        aPinNum=j;
        aPinType=QPT_INPUT;
        aFilter=c;
        return;
      }
    }
  
    for (int j=0; j<c->fOutputs.count(); j++)
    {
      QCPOutput* a=c->fOutputs[j];
      p=QPoint(cpOwner->mapToGlobal(QPoint(a->ownerFilter->x()+a->relCoord.x()+QCP_PINSIZE/2,a->ownerFilter->y()+a->relCoord.y()+QCP_PINSIZE/2)));
      r=QRect(p.x()-QCP_PINSIZE,p.y()-QCP_PINSIZE,2*QCP_PINSIZE,2*QCP_PINSIZE);
      if (r.contains(mx.x(),mx.y()))
      {
        aPinNum=j;
        aPinType=QPT_OUTPUT;
        aFilter=c;
        return;
      }
    }
  }
}

void QCPBase::redrawPins(QPainter & painter)
{
  if (cpOwner->resReading) return;
  realignPins(painter);

  QPen op=painter.pen();
  QBrush ob=painter.brush();
  QFont of=painter.font();
  QPen penPin=QPen(pinColor);
  QBrush brshPin=QBrush(pinColor,Qt::SolidPattern);
  painter.setPen(penPin);
  painter.setBrush(brshPin);
  QFont n=of;
  n.setBold(false);
  painter.setFont(n);
  for (int i=0;i<fInputs.count();i++)
  {
    QCPInput* a=fInputs[i];
    painter.fillRect(QRect(   a->relCoord.x()-QCP_PINSIZE/2,
                              a->relCoord.y()-QCP_PINSIZE/2,
                              QCP_PINSIZE,
                              QCP_PINSIZE),brshPin);
    painter.drawText(QPoint(  a->relCoord.x()+QCP_PINSIZE/2+1,
                              a->relCoord.y()+painter.fontMetrics().height()/4),
                            a->pinName);
    
  }
  for (int i=0;i<fOutputs.count();i++)
  {
    QCPOutput* a=fOutputs[i];
    painter.fillRect(QRect(   a->relCoord.x()-QCP_PINSIZE/2,
                              a->relCoord.y()-QCP_PINSIZE/2,
                              QCP_PINSIZE,
                              QCP_PINSIZE),brshPin);
    painter.drawText(QPoint(  a->relCoord.x()-QCP_PINSIZE/2-1 - painter.fontMetrics().width(a->pinName),
                              a->relCoord.y()+painter.fontMetrics().height()/4), a->pinName);
  }
  painter.setFont(of);
  painter.setBrush(ob);
  painter.setPen(op);
}

void QCPBase::postLoadBind()
{
  for (int i=0;i<fInputs.count();i++)
    fInputs[i]->postLoadBind();
  for (int i=0;i<fOutputs.count();i++)
    fOutputs[i]->postLoadBind();
}

void QCPBase::checkRecycle()
{
  if (!(frameGeometry().intersects(cpOwner->recycle->frameGeometry()))) return;
  for (int i=0;i<fInputs.count();i++)
  {
    if (fInputs[i]->fromPin!=-1)
      if (fInputs[i]->fromFilter!=0)
      {
        fInputs[i]->fromFilter->fOutputs[fInputs[i]->fromPin]->toFilter=0;
        fInputs[i]->fromFilter->fOutputs[fInputs[i]->fromPin]->toPin=-1;
      }
    fInputs[i]->fromFilter=0;
    fInputs[i]->fromPin=-1;
  }
  for (int i=0;i<fOutputs.count();i++)
  {
    if (fOutputs[i]->toPin!=-1)
      if (fOutputs[i]->toFilter!=0)
      {
        fOutputs[i]->toFilter->fInputs[fOutputs[i]->toPin]->fromFilter=0;
        fOutputs[i]->toFilter->fInputs[fOutputs[i]->toPin]->fromPin=-1;
      }
    fOutputs[i]->toFilter=0;
    fOutputs[i]->toPin=-1;
  }
  cpOwner->repaintConn();
  deleteLater();
}

void QCPBase::mouseMoveEvent(QMouseEvent * event)
{
//  if (event->button()==Qt::LeftButton)
  
    if (isDragging)
    {
      move(QPoint(x()+event->x()-relCorner.x(),y()+event->y()-relCorner.y()));
      cpOwner->repaintConn();
      cpOwner->resize(cpOwner->minimumSizeHint());
      update();
      emit componentChanged(this);
    } else
      cpOwner->refreshConnBuilder(event->pos());
  
}

void QCPBase::paintEvent ( QPaintEvent * )
{
  QPainter p(this);
  p.setPen(QPen(Qt::black));
  p.setBrush(QBrush(Qt::white,Qt::SolidPattern));
  
  p.drawRect(rect());
}

void QCPBase::mousePressEvent(QMouseEvent * event)
{
  raise();
  if (event->button()==Qt::RightButton)
  {
    fSettingsDlg=true;
    showSettingsDlg();
    fSettingsDlg=false;
    return;
  }
  QPoint mx=mapToGlobal(event->pos());
  QCPBase* dFlt;
  int pNum,pType;
  mouseInPin(mx,pNum,pType,dFlt);
  emit componentChanged(this);
  if (pNum==-1)
  {
    relCorner=event->pos();
    isDragging=true;
    return;
  }
  isDragging=false;
  if (pType==QPT_INPUT)
    cpOwner->initConnBuilder(QPT_INPUT,pNum,dFlt->fInputs[pNum],0);
  else
    cpOwner->initConnBuilder(QPT_OUTPUT,pNum,0,dFlt->fOutputs[pNum]);
}

void QCPBase::mouseReleaseEvent(QMouseEvent * event)
{
  if (fSettingsDlg)
  {
    fSettingsDlg=false;
    return;
  }
  QPoint mx=mapToGlobal(event->pos());
  QCPBase* dFlt;
  int pNum,pType;
  mouseInPin(mx,pNum,pType,dFlt);
  if (pNum==-1)
  {
    bool f=isDragging;
    isDragging=false;
    if (!f)
      cpOwner->doneConnBuilder(true,QPT_INPUT,-1,0,0);
    else
      checkRecycle();
    isDragging=f;
    return;
  }
  if (pType==QPT_INPUT)
    cpOwner->doneConnBuilder(false,QPT_INPUT,pNum,dFlt->fInputs[pNum],0);
  else
    cpOwner->doneConnBuilder(false,QPT_OUTPUT,pNum,0,dFlt->fOutputs[pNum]);
  emit componentChanged(this);
}

void QCPBase::readFromStream( QDataStream & stream )
{
  stream >> pinColor;
  for (int i=0;i<fInputs.count();i++)
    fInputs[i]->readFromStream(stream);
  for (int i=0;i<fOutputs.count();i++)
    fOutputs[i]->readFromStream(stream);
}

void QCPBase::storeToStream( QDataStream & stream )
{
  stream << pinColor;
  for (int i=0;i<fInputs.count();i++)
    fInputs[i]->storeToStream(stream);
  for (int i=0;i<fOutputs.count();i++)
    fOutputs[i]->storeToStream(stream);
}

void QCPBase::doGenerate(QTextStream & stream)
{
  if (cpOwner->erroneousRoute) return;
  if (cpOwner->nodeLocks.contains(objectName()))
  {
    cpOwner->erroneousRoute=true;
    return;
  }
  cpOwner->nodeLocks.append(objectName());
  doInfoGenerate(stream);
  cpOwner->nodeLocks.removeAt(cpOwner->nodeLocks.indexOf(QRegExp(objectName())));
}

void QCPBase::showSettingsDlg()
{
}

QCPOutput::QCPOutput(QObject * parent, QCPBase * aOwner)
  : QObject(parent)
{
  pinName="<NA>";
  toPin=-1;
  toFilter=0;
  relCoord=QPoint(0,0);
  ownerFilter=aOwner;
}

void QCPOutput::readFromStream( QDataStream & stream )
{
  stream >> toPin;
  toFilter=0;
  QString q;
  stream >> q;
  if (q!="<NONE>")
    ffLogic=q;
  else
    ffLogic="";
}

void QCPOutput::storeToStream( QDataStream & stream )
{
  stream << toPin;
  QString q;
  if (toFilter==0)
    q="<NONE>";
  else
    q=toFilter->objectName();
  stream << q;
}

void QCPOutput::postLoadBind()
{
  if (ffLogic=="") return;
  QCPBase* b=ownerFilter->cpOwner->findChild<QCPBase *>(ffLogic);
  if (b==0)
    throw "Binding error. Error in loading components from file.";
  else
    toFilter=b;
}

QCPInput::QCPInput(QObject * parent, QCPBase * aOwner)
  : QObject(parent)
{
  pinName="<NA>";
  fromPin=-1;
  fromFilter=0;
  relCoord=QPoint(0,0);
  ownerFilter=aOwner;
}

void QCPInput::readFromStream( QDataStream & stream )
{
  stream >> fromPin;
  fromFilter=0;
  QString q;
  stream >> q;
  if (q!="<NONE>")
    ffLogic=q;
  else
    ffLogic="";
}

void QCPInput::storeToStream( QDataStream & stream )
{
  stream << fromPin;
  QString q;
  if (fromFilter==0)
    q="<NONE>";
  else
    q=fromFilter->objectName();
  stream << q;
}

void QCPInput::postLoadBind()
{
  if (ffLogic=="") return;
  QCPBase* b=ownerFilter->cpOwner->findChild<QCPBase *>(ffLogic);
  if (b==0)
    throw "Binding error. Error in loading components from file.";
  else
    fromFilter=b;
}

void debugPrint(const QString &s)
{
  QByteArray a(s.toUtf8());
  a+="\n";
  printf(a.data());
}
