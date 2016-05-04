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
#include "includes/cpconv.h"
#include "includes/qconvdialog.h"

QCPConv::QCPConv(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  fOut=new QCPOutput(this,this);
  fOut->pinName="out";
  fOutputs.append(fOut);
  alConverter=alcLinear;
  alFormat="S16_LE";
}

QCPConv::~QCPConv()
{
  delete fInp;
  delete fOut;
}

QSize QCPConv::minimumSizeHint() const
{
  return QSize(180,50);
}

QSize QCPConv::sizeHint() const
{
  return minimumSizeHint();
}

void QCPConv::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
  fOut->relCoord=QPoint(width()-QCP_PINSIZE/2,height()/2);
}

void QCPConv::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  QString conv="linear";
  switch (alConverter) {
    case alcLinear: conv="linear"; break;
    case alcFloat: conv="lfloat"; break;
    case alcIEC958: conv="iec958"; break;
    case alcMuLaw: conv="mulaw"; break;
    case alcALaw: conv="alaw"; break;
    case alcImaADPCM: conv="adpcm"; break;
  }
  stream << "  type " << conv << endl;
  if (fOut->toFilter!=0)
  {
    stream << "  slave {" << endl;
    stream << "    pcm \"" << fOut->toFilter->objectName() << "\"" << endl;
    if (alConverter!=alcIEC958)
      stream << "    format " << alFormat << endl;
    stream << "  }" << endl;
  }
  stream << "}" << endl;
  stream << endl;
  if (fOut->toFilter!=0)
    fOut->toFilter->doGenerate(stream);
}

void QCPConv::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"Converter");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
    
  QString conv="linear";
  switch (alConverter) {
    case alcLinear: conv="linear"; break;
    case alcFloat: conv="linear<->float"; break;
    case alcIEC958: conv="linear<->IEC958 frames"; break;
    case alcMuLaw: conv="linear<->MuLaw"; break;
    case alcALaw: conv="linear<->ALaw"; break;
    case alcImaADPCM: conv="linear<->ImaADPCM"; break;
  }
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,conv);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPConv::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream.readRawData((char*)&alConverter,sizeof(alConverter));
  stream >> alFormat;
}

void QCPConv::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream.writeRawData((char*)&alConverter,sizeof(alConverter));
  stream << alFormat;
}

void QCPConv::showSettingsDlg()
{
  QConvDialog* d=new QConvDialog();
  QString conv="linear";
  switch (alConverter) {
    case alcLinear: conv="linear"; break;
    case alcFloat: conv="linear<->float"; break;
    case alcIEC958: conv="linear<->IEC958 frames"; break;
    case alcMuLaw: conv="linear<->MuLaw"; break;
    case alcALaw: conv="linear<->ALaw"; break;
    case alcImaADPCM: conv="linear<->ImaADPCM"; break;
  }
  d->alConverter->setText(conv);
  
  int fmtidx=d->alFormat->findText(alFormat,Qt::MatchStartsWith | Qt::MatchCaseSensitive);
  if ((fmtidx>=0) && (fmtidx<d->alFormat->count()))
    d->alFormat->setCurrentIndex(fmtidx);

  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  emit componentChanged(this);

  alFormat=d->alFormat->currentText().section(" ",0,0);
  delete d;
  update();
}
