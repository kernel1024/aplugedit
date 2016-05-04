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
#include "includes/renderarea.h"
#include "includes/cpinp.h"

static int dspCnt=-1;

QCPInp::QCPInp(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  if (dspCnt==-1)
    dspNum="!default";
  else
    dspNum="dsp"+QString::number(dspCnt++);
  if (dspCnt>7) dspCnt=-1;
}

QCPInp::~QCPInp()
{
  delete fOut;
}

void QCPInp::showSettingsDlg()
{
  QStringList items;
  items << "!default" << "dsp0" << "dsp1" << "dsp2" << "dsp3" << "dsp4" << "dsp5" << "dsp6" << "dsp7";
  int c=0;
  if (!(items.contains(dspNum)))
  {
    items << dspNum;
    c=items.count()-1;
  } else
    c=items.indexOf(QRegExp(dspNum));
  
  bool ok;
  QString item = QInputDialog::getItem(this, tr("DSP input device"),
                                        tr("User-land device number"), items, c, true, &ok);
  if (ok && !item.isEmpty())
  {
    dspNum=item;
    for (int i=0;i<cpOwner->children().count();i++)
      if (QCPInp *base=qobject_cast<QCPInp*>(cpOwner->children().at(i)))
        if ((base->objectName()!=objectName()) && (base->dspNum==dspNum))
        {
          QMessageBox::warning(0,tr("Duplicated DSP"),tr("You have entered duplicated identifier for this DSP input,\nthat is already used in another component.\nPlease, recheck your DSP inputs!"));
          break;
        }
    update();
    emit componentChanged(this);
  }
}

void QCPInp::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> dspNum;
}

void QCPInp::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << dspNum;
}

QSize QCPInp::minimumSizeHint() const
{
  return QSize(180,50);
}

QSize QCPInp::sizeHint() const
{
  return minimumSizeHint();
}

void QCPInp::realignPins(QPainter &)
{
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPInp::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << dspNum << " {" << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  type plug" << endl;
    stream << "  slave.pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
  } else {
    stream << "  type hw" << endl;
    stream << "  card 0" << endl;
  }
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPInp::paintEvent ( QPaintEvent *)
{
  QPainter p(this);
  QPen op=p.pen();
  QBrush ob=p.brush();
  QFont of=p.font();
  QPen pn=QPen(Qt::black);
  pn.setWidth(2);
  p.setPen(pn);
  p.setBrush(QBrush(Qt::white,Qt::SolidPattern));
  
  p.drawRect(rect());
  
  redrawPins(p);
  
  QFont n=of;
  n.setBold(true);
  n.setPointSize(n.pointSize()+1);
  p.setFont(n);
  p.drawText(rect(),Qt::AlignCenter,"Input DSP");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,dspNum);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}
