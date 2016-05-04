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
#include "includes/cpnull.h"

QCPNull::QCPNull(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
}

QCPNull::~QCPNull()
{
  delete fInp;
}

QSize QCPNull::minimumSizeHint() const
{
  return QSize(150,50);
}

QSize QCPNull::sizeHint() const
{
  return minimumSizeHint();
}

void QCPNull::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
}

void QCPNull::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type null" << endl;
  stream << "}" << endl;
  stream << endl;
}

void QCPNull::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"null");
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

