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
#include "includes/cprate.h"
#include "includes/qratedialog.h"

QCPRate::QCPRate(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  alRate=48000;
  alConverter="";
}

QCPRate::~QCPRate()
{
  delete fInp;
  delete fOut;
}

QSize QCPRate::minimumSizeHint() const
{
  return QSize(140,50);
}

QSize QCPRate::sizeHint() const
{
  return minimumSizeHint();
}

void QCPRate::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPRate::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type rate" << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    stream << "    rate " << alRate << endl;
    stream << "  }" << endl;
  }
  if (alConverter!="")
    stream << "  converter \"" << alConverter << "\"" << endl;
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPRate::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"SRC");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  QString s=QString::number(alRate)+" Hz";
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,s);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPRate::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> alRate;
  stream >> alConverter;
}

void QCPRate::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << alRate;
  stream << alConverter;
}

void QCPRate::showSettingsDlg()
{
  QRateDialog* d=new QRateDialog();
  d->setParams(alRate,alConverter);

  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  emit componentChanged(this);
  
  alRate=d->getRate();
  alConverter=d->getConverter();
  delete d;
  update();
}

