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

#include "includes/cpbase.h"
#include "includes/cpmeter.h"
#include "includes/qmeterdialog.h"

QCPMeter::QCPMeter(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  alMeterLib="";
  alMeterFunc="";
  alRefreshRate=50;
}

QCPMeter::~QCPMeter()
{
  delete fInp;
  delete fOut;
}

QSize QCPMeter::minimumSizeHint() const
{
  return QSize(180,50);
}

QSize QCPMeter::sizeHint() const
{
  return minimumSizeHint();
}

void QCPMeter::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPMeter::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm_scope_type." << objectName() << " {" << endl;
  if (alMeterLib!="")
    stream << "  lib \"" << alMeterLib << "\"" << endl;
  if (alMeterFunc!="")
    stream << "  open \"" << alMeterFunc << "\"" << endl;
  stream << "}" << endl;
  stream << endl;
  
  stream << "pcm_scope." << objectName() << " {" << endl;
  stream << "  type " << objectName() << endl;
  stream << "}" << endl;
  stream << endl;
  
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type meter" << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    stream << "  }" << endl;
  }
  stream << "  frequency " << alRefreshRate << endl;
  stream << "  scopes.0 " << objectName() << endl;
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPMeter::paintEvent ( QPaintEvent * )
{
  QPainter p(this);
  QPen pn=QPen(Qt::black);
  QPen op=p.pen();
  QBrush ob=p.brush();
  QFont of=p.font();
  pn.setWidth(2);
  p.setPen(pn);
  p.setBrush(QBrush(Qt::white,Qt::SolidPattern));
  
  p.drawRect(rect());
  
  redrawPins(p);
  
  QFont n=of;
  n.setBold(true);
  n.setPointSize(n.pointSize()+1);
  p.setFont(n);
  p.drawText(rect(),Qt::AlignCenter,"VU Meter");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
    
  QFileInfo fi(alMeterLib);
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,fi.fileName());
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPMeter::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> alMeterLib;
  stream >> alMeterFunc;
  stream >> alRefreshRate;
}

void QCPMeter::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << alMeterLib;
  stream << alMeterFunc;
  stream << alRefreshRate;
}

void QCPMeter::showSettingsDlg()
{
  QMeterDialog* d=new QMeterDialog();
  d->alMeterLib->setText(alMeterLib);
  d->alMeterFunc->setText(alMeterFunc);
  d->alRefreshRate->setValue(alRefreshRate);

  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  emit componentChanged(this);

  alMeterLib=d->alMeterLib->text();
  alMeterFunc=d->alMeterFunc->text();
  alRefreshRate=d->alRefreshRate->value();
  delete d;
  update();
}

