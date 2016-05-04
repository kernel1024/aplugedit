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
#include "includes/cpdmix.h"
#include "includes/cphw.h"

QCPDMix::QCPDMix(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
}

QCPDMix::~QCPDMix()
{
  delete fInp;
  delete fOut;
}

QSize QCPDMix::minimumSizeHint() const
{
  return QSize(140,50);
}

QSize QCPDMix::sizeHint() const
{
  return minimumSizeHint();
}

void QCPDMix::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPDMix::doInfoGenerate(QTextStream & stream)
{
  QCPHW *hw=qobject_cast<QCPHW*>(fOut->toFilter);
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type dmix" << endl;
  stream << "  ipc_key " << rand() << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    if (hw)
    {
      if (hw->alChannels!=-1)
        stream << "    channels " << hw->alChannels << endl;
      if (hw->alRate!=-1)
        stream << "    rate " << hw->alRate << endl;
    }
    stream << "  }" << endl;
    if (hw)
      if (hw->alChannels!=-1)
      {
        stream << "  bindings {" << endl;
        for (int i=0;i<hw->alChannels;i++)
          stream << "    " << i << " " << i << endl;
        stream << "  }" << endl;
      }
  }
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

bool QCPDMix::canConnectOut(QCPBase * toFilter)
{
  QCPHW *base=qobject_cast<QCPHW*>(toFilter);
  return (base!=0);
}

void QCPDMix::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"DMix");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}
