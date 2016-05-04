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
#include "includes/cproute.h"
#include "includes/qroutedialog.h"

QCPRoute::QCPRoute(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  alChannelsIn=2;
  alChannelsOut=2;
  alTable[0].from=0;
  alTable[0].coeff=1.0;
  alTable[1].from=1;
  alTable[1].coeff=1.0;
}

QCPRoute::~QCPRoute()
{
  delete fInp;
  delete fOut;
}

QSize QCPRoute::minimumSizeHint() const
{
  return QSize(140,50);
}

QSize QCPRoute::sizeHint() const
{
  return minimumSizeHint();
}

void QCPRoute::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPRoute::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type route" << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    stream << "    channels " << alChannelsOut << endl;
    stream << "  }" << endl;
  }
  for (int i=0;i<alChannelsOut;i++)
  {
    QString s=QString::number(alTable[i].coeff,'f',1);
    stream << "  ttable." << alTable[i].from << "." << i << " " << s << endl;
  }
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPRoute::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"Route");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  QString s=tr("%1 -> %2").arg(alChannelsIn).arg(alChannelsOut);
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,s);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPRoute::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> alChannelsIn;
  stream >> alChannelsOut;
  stream.readRawData((char*)&alTable,sizeof(alTable));
}

void QCPRoute::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << alChannelsIn;
  stream << alChannelsOut;
  stream.writeRawData((char*)&alTable,sizeof(alTable));
}

void QCPRoute::showSettingsDlg()
{
  QRouteDialog* d=new QRouteDialog();
  d->setParams(alChannelsIn,alChannelsOut,alTable);

  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  emit componentChanged(this);

  alChannelsIn=d->getInChannels();
  alChannelsOut=d->getOutChannels();
  d->getTable(alTable);
  delete d;
  update();
}

