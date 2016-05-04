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

#include "includes/renderarea.h"
#include "includes/mainwindow.h"

#include "includes/cpinp.h"
#include "includes/cphw.h"
#include "includes/cpnull.h"
#include "includes/cpfile.h"
#include "includes/cprate.h"
#include "includes/cproute.h"
#include "includes/cpdmix.h"
#include "includes/cpmeter.h"
#include "includes/cpconv.h"
#include "includes/cpladspa.h"

QRenderArea::QRenderArea(QWidget *parent, QScrollArea *aScroller)
    : QFrame(parent)
{
  scroller=aScroller;
  
  resReading=false;
  erroneousRoute=false;
  rectLinks=true;
  nodeLocks.clear();
  
  recycle=new QLabel(this);
  recycle->setGeometry(QRect(10, 10, 48, 48));
  recycle->setPixmap(QPixmap(":/images/trashcan_empty.png"));
  recycle->setText("");
  recycle->setObjectName("trashcan");
  
  cbType=0;
  cbPinNum=-1;
  cbConnCount=0;
  cbInput=0;
  cbOutput=0;
  cbBuilding=false;
  cbCurrent=QPoint(0,0);
}

QSize QRenderArea::minimumSizeHint() const
{
  int x=3*scroller->width()/2;
  int y=3*scroller->height()/2;
  QRect r(0,0,0,0);
  for (int i=0;i<children().count();i++)
    if (QWidget* w=qobject_cast<QWidget*>(children().at(i)))
      r=r.unite(w->geometry());
  QSize cmSize=QSize(r.width(),r.height());
  return QSize(x,y).expandedTo(cmSize);
}

QSize QRenderArea::sizeHint() const
{
  return minimumSizeHint();
}

void QRenderArea::initConnBuilder(const int aType, int aPinNum, QCPInput* aInput, QCPOutput* aOutput)
{
  if (cbBuilding)
  {
    cbBuilding=false;
    return;
  }
  cbType=aType;
  cbPinNum=aPinNum;
  cbInput=0;
  cbOutput=0;
  // init connection form input
  if (cbType==QPT_INPUT)
  {
    cbInput=aInput;
    if (cbInput!=0)
    {
      // disconnect old connections from this input
      if ((cbInput->fromPin!=-1) && (cbInput->fromFilter!=0))
      {
        cbInput->fromFilter->fOutputs[cbInput->fromPin]->toFilter=0;
        cbInput->fromFilter->fOutputs[cbInput->fromPin]->toPin=-1;
      }
      cbInput->fromFilter=0;
      cbInput->fromPin=-1;
      cbCurrent=cbInput->ownerFilter->pos()+cbInput->relCoord;
    }
  } else {
    // init connection from output
    cbOutput=aOutput;
    if (cbOutput!=0)
    {
      // disconnect old connections from this output
      if ((cbOutput->toPin!=-1) && (cbOutput->toFilter!=0))
      {
        cbOutput->toFilter->fInputs[cbOutput->toPin]->fromFilter=0;
        cbOutput->toFilter->fInputs[cbOutput->toPin]->fromPin=-1;
      }
      cbOutput->toFilter=0;
      cbOutput->toPin=-1;
      cbCurrent=cbOutput->ownerFilter->pos()+cbOutput->relCoord;
    }
  }
  cbBuilding=true;
}

void QRenderArea::repaintConn()
{
  repaint();
}

void QRenderArea::paintEvent ( QPaintEvent * event )
{
  QPainter p(this);
  cbConnCount=0;
  QCPOutput* aout;
  QCPInput* ainp;
  QPoint c1,c2,c3,c4;
  
  if (event->isAccepted());
  
  QPen op=p.pen();
  p.setPen(QPen(Qt::red));
  cbConnCount=0;
  for (int i=0;i<children().count();i++)
  {
    QCPBase *base=qobject_cast<QCPBase*>(children().at(i));
    if (base==0) continue;
    for (int j=0;j<base->fOutputs.count();j++)
    {
      aout=base->fOutputs[j];
      if (aout->toPin==-1) continue;
      if (aout->toFilter==0) continue;
      ainp=aout->toFilter->fInputs[aout->toPin];
      
      c1=base->pos()+aout->relCoord;
      c2=aout->toFilter->pos()+ainp->relCoord;
      c3=QPoint((c1.x()+c2.x())/2,c1.y());
      c4=QPoint(c3.x(),c2.y());
      
      if (rectLinks)
      {
        p.drawLine(c1,c3);
        p.drawLine(c3,c4);
        p.drawLine(c4,c2);
      } else
        p.drawLine(c1,c2);
      
      cbConnCount++;
    }
  }
  if (cbBuilding)
  {
    p.setPen(QPen(Qt::darkCyan));
    if (cbType==QPT_INPUT)
      c1=cbInput->ownerFilter->pos()+cbInput->relCoord;
    else
      c1=cbOutput->ownerFilter->pos()+cbOutput->relCoord;
    c2=cbCurrent;
    p.drawLine(c1,c2);
  }
  p.setPen(op);
}


void QRenderArea::refreshConnBuilder(const QPoint & atPos)
{
  if (!cbBuilding) return;
  if (cbType==QPT_INPUT)
    cbCurrent=QPoint(cbInput->ownerFilter->pos()+atPos);
  else
    cbCurrent=QPoint(cbOutput->ownerFilter->pos()+atPos);
  repaintConn();
}

void QRenderArea::doneConnBuilder(const bool aNone, int aType, const int aPinNum, QCPInput* aInput, QCPOutput* aOutput)
{
  // if we making trace from input to this output...
  if ((cbType==QPT_INPUT) && (cbInput!=0))
  {
    // and we have new output now...
    if (aOutput!=0)
    {
      // then we remove old connection to this output to connect our new trace to it
      if ((aOutput->toPin!=-1) && (aOutput->toFilter!=0))
      {
        aOutput->toFilter->fInputs[aOutput->toPin]->fromFilter=0;
        aOutput->toFilter->fInputs[aOutput->toPin]->fromPin=-1;
      }
      aOutput->toFilter=0;
      aOutput->toPin=-1;
    }
    // if our input (from that we making connection) is connected - then disconnect it now
    if ((cbInput->fromPin!=-1) && (cbInput->fromFilter!=0))
    {
      cbInput->fromFilter->fOutputs[cbInput->fromPin]->toFilter=0;
      cbInput->fromFilter->fOutputs[cbInput->fromPin]->toPin=-1;
    }
    cbInput->fromFilter=0;
    cbInput->fromPin=-1;
  }
  // if we making trace from output to this input...
  else if (cbOutput!=0)
  {
    // and we have new input now...
    if (aInput!=0)
    {
      // then we remove old connection to this input
      if ((aInput->fromPin!=-1) && (aInput->fromFilter!=0))
      {
        aInput->fromFilter->fOutputs[aInput->fromPin]->toFilter=0;
        aInput->fromFilter->fOutputs[aInput->fromPin]->toPin=-1;
      }
      aInput->fromFilter=0;
      aInput->fromPin=-1;
    }
    // if our output (from that we making connection) is connected - then disconnect it now
    if ((cbOutput->toPin!=-1) && (cbOutput->toFilter!=0))
    {
      cbOutput->toFilter->fInputs[cbOutput->toPin]->fromFilter=0;
      cbOutput->toFilter->fInputs[cbOutput->toPin]->fromPin=-1;
    }
    cbOutput->toFilter=0;
    cbOutput->toPin=-1;
  }
  // if this is simple deletion or incorrect route (in-in, out-out), then delete it
  if ((aNone) || (aType==cbType))
  {
    cbBuilding=false;
    repaintConn();
    return;
  }
  // if this output can't possible connect to specified input (np: DMix connecting not to HW), then delete it
  QCPBase *aTo, *aFrom;
  if (cbType==QPT_INPUT)
  {
    aTo=cbInput->ownerFilter;
    aFrom=aOutput->ownerFilter;
  } else {
    aTo=aInput->ownerFilter;
    aFrom=cbOutput->ownerFilter;
  }
  if ((!aFrom->canConnectOut(aTo)) || (!aTo->canConnectIn(aFrom)))
  {
    cbBuilding=false;
    repaintConn();
    return;
  }
  if (cbType==QPT_INPUT)
  {
    cbInput->fromFilter=aOutput->ownerFilter;
    cbInput->fromPin=aPinNum;
    aOutput->toFilter=cbInput->ownerFilter;
    aOutput->toPin=cbPinNum;
  } else {
    cbOutput->toFilter=aInput->ownerFilter;
    cbOutput->toPin=aPinNum;
    aInput->fromFilter=cbOutput->ownerFilter;
    aInput->fromPin=cbPinNum;
  }
  cbBuilding=false;
  repaintConn();
}

void QRenderArea::postLoadBinding()
{
  for (int i=0;i<children().count();i++)
  {
    QCPBase* base=qobject_cast<QCPBase*>(children().at(i));
    if (base!=0) base->postLoadBind();
  }
}

void QRenderArea::doGenerate(QTextStream & stream)
{
  // Generate header
  stream << "# This file is automatically generated in ALSA Plugin Editor." << endl;
  
  // Generate plugin tree for all DSP inputs
  for (int i=0;i<children().count();i++)
    if (QCPInp *base=qobject_cast<QCPInp*>(children().at(i)))
    {
      stream << "# plugin tree for " << base->objectName() << endl;
      base->doGenerate(stream);
    }
    
  // Finalizing config
  stream << "# Generation successfully completted." << endl;
}

int QRenderArea::cpComponentCount()
{
  int c=0;
  QCPBase* base;
  for (int i=0;i<children().count();i++)
    if (base=qobject_cast<QCPBase*>(children().at(i))) c++;
  if (base==0);
  return c;
}

void QRenderArea::readSchematic(QDataStream & stream)
{
  int c;
  stream >> c;
  for (int i=0;i<c;i++)
  {
    QString clName;
    stream >> clName;
    QCPBase* b=createCpInstance(clName);
    if (b==0)
    {
      debugPrint(clName);
      throw "Loading error. Class not found.";
      return;
    }
    QPoint pos;
    QString instName;
    stream >> pos;
    stream >> instName;
    b->move(pos);
    b->setObjectName(instName);
    b->readFromStream(stream);
    b->show();
  }
  resReading=false;
  postLoadBinding();
  repaintConn();
}

void QRenderArea::storeSchematic(QDataStream & stream)
{
  int c=cpComponentCount();
  stream << c;
  for (int i=0;i<children().count();i++)
  {
    QCPBase* base=qobject_cast<QCPBase*>(children().at(i));
    if (!base) continue;
    QString a=base->metaObject()->className();
    QPoint p=base->pos();
    QString n=base->objectName();
    stream << a;
    stream << p;
    stream << n;
    base->storeToStream(stream);
  }
}

void QRenderArea::deleteComponents()
{
  for (int i=0;i<children().count();i++)
  {
    QCPBase* base=qobject_cast<QCPBase*>(children().at(i));
    if (base!=0) base->deleteLater();
  }
}

QCPBase* QRenderArea::createCpInstance(QString & className)
{
  if (className=="QCPInp")      return new QCPInp(this,this);
  else if (className=="QCPHW")  return new QCPHW(this,this);
  else if (className=="QCPNull")  return new QCPNull(this,this);
  else if (className=="QCPFile")  return new QCPFile(this,this);
  else if (className=="QCPRate")  return new QCPRate(this,this);
  else if (className=="QCPRoute")  return new QCPRoute(this,this);
  else if (className=="QCPDMix")  return new QCPDMix(this,this);
  else if (className=="QCPMeter")  return new QCPMeter(this,this);
  else if (className=="QCPConv")  return new QCPConv(this,this);
  else if (className=="QCPLADSPA")  return new QCPLADSPA(this,this);
  else return 0;
}
