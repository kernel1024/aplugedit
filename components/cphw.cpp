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
#include "includes/cphw.h"
#include "includes/qhwdialog.h"

QCPHW::QCPHW(QWidget *parent, QRenderArea *aOwner)
  : QCPBase(parent,aOwner)
{
  fInp=new QCPInput(this,this);
  fInp->pinName="in";
  fInputs.append(fInp);
  alCard=0;
  alDevice=-1;
  alSubdevice=-1;
  alMmap_emulation=-1;
  alSync_ptr_ioctl=-1;
  alNonblock=-1;
  alFormat="<NONE>";
  alChannels=-1;
  alRate=-1;
}

QCPHW::~QCPHW()
{
  delete fInp;
}

QSize QCPHW::minimumSizeHint() const
{
  return QSize(180,50);
}

QSize QCPHW::sizeHint() const
{
  return minimumSizeHint();
}

void QCPHW::realignPins(QPainter &)
{
  fInp->relCoord=QPoint(QCP_PINSIZE/2,height()/2);
}

void QCPHW::doInfoGenerate(QTextStream & stream)
{
  stream << "pcm." << objectName() << " {" << endl;
  stream << "  type hw" << endl;
  if (alCard==-1) 
    stream << "  pcm \"hw:0,0\"" << endl;
  else
  {
    stream << "  card " << alCard << endl;
    if (alDevice!=-1) 
    {
      stream << "  device " << alDevice << endl;
      if (alSubdevice!=-1) 
        stream << "  subdevice " << alSubdevice << endl;
    }
  }
  if (alMmap_emulation==0) 
    stream << "  mmap_emulation false" << endl;
  else if (alMmap_emulation==1) 
    stream << "  mmap_emulation true" << endl;
  
  if (alSync_ptr_ioctl==0) 
    stream << "  sync_ptr_ioctl false" << endl;
  else if (alSync_ptr_ioctl==1) 
    stream << "  sync_ptr_ioctl true" << endl;
  
  if (alNonblock==0) 
    stream << "  nonblock false" << endl;
  else if (alNonblock==1) 
    stream << "  nonblock true" << endl;

  if (alFormat!="<NONE>")
    stream << "  format " << alFormat << endl;
  
  if (alChannels!=-1)
    stream << "  channels " << alChannels << endl;
  
  if (alRate!=-1)
    stream << "  rate " << alRate << endl;
  stream << "}" << endl;
  stream << endl;
}

void QCPHW::readFromStream( QDataStream & stream )
{
  QCPBase::readFromStream(stream);
  stream >> alCard >> alDevice >> alSubdevice;
  stream >> alMmap_emulation >> alSync_ptr_ioctl >> alNonblock;
  stream >> alFormat;
  stream >> alChannels >> alRate;
}

void QCPHW::storeToStream( QDataStream & stream )
{
  QCPBase::storeToStream(stream);
  stream << alCard << alDevice << alSubdevice;
  stream << alMmap_emulation << alSync_ptr_ioctl << alNonblock;
  stream << alFormat;
  stream << alChannels << alRate;
}

void QCPHW::paintEvent ( QPaintEvent * )
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
  p.drawText(rect(),Qt::AlignCenter,"HW output");
  
  n.setBold(false);
  n.setPointSize(n.pointSize()-3);
  p.setPen(QPen(Qt::gray));
  p.setFont(n);
  QString rate;
  if (alRate==-1)
    rate="* Hz";
  else if (alRate<10000)
    rate=QString::number(alRate)+" Hz";
  else
    rate=QString::number(((float)alRate)/1000,'f',1)+"kHz";
  QString s=tr("hw:%1,%2 ch:%3, %4")
              .arg(alCard)
              .arg(alDevice)
              .arg(alChannels)
              .arg(rate);
  p.drawText(QRect(0,height()/3,width(),height()),Qt::AlignCenter,s);
  
  p.setFont(of);
  p.setBrush(ob);
  p.setPen(op);
}

void QCPHW::showSettingsDlg()
{
  QHWDialog* d=new QHWDialog();
  int cardidx=-1;
  int devidx=-1;
  int subidx=-1;
  for (int i=0;i<d->hwCnt.count();i++)
    if (d->hwCnt[i]->cardNum==alCard)
    {
      cardidx=i;
      break;
    }
  if (cardidx!=-1)
  {
    d->alCard->setCurrentIndex(cardidx+1);
    d->cardSelected(cardidx+1);
    for (int i=0;i<d->hwCnt[cardidx]->devices.count();i++)
      if (d->hwCnt[cardidx]->devices[i]->devNum==alDevice)
      {
        devidx=i;
        break;
      }
    if (devidx!=-1)
    {
      d->alDevice->setCurrentIndex(devidx+1);
      d->devSelected(devidx+1);
      if ((alSubdevice<d->hwCnt[cardidx]->devices[devidx]->subdevices) && (alSubdevice>=0))
      {
        subidx=alSubdevice;
        d->alSubdevice->setCurrentIndex(subidx+1);
      }
    }
  }
  
  int fmtidx=d->alFormat->findText(alFormat,Qt::MatchStartsWith | Qt::MatchCaseSensitive);
  if ((fmtidx>=0) && (fmtidx<d->alFormat->count()))
    d->alFormat->setCurrentIndex(fmtidx);
  
  switch (alChannels)
  {
    case 1:
      d->alChannels->setCurrentIndex(1);
      break;
    case 2:
      d->alChannels->setCurrentIndex(2);
      break;
    case 4:
      d->alChannels->setCurrentIndex(3);
      break;
    case 6:
      d->alChannels->setCurrentIndex(4);
      break;
    case 8:
      d->alChannels->setCurrentIndex(5);
      break;
    default:
      d->alChannels->setCurrentIndex(0);
  }
  
  switch (alRate)
  {
    case 8000:
      d->alRate->setCurrentIndex(1);
      break;
    case 11025:
      d->alRate->setCurrentIndex(2);
      break;
    case 22050:
      d->alRate->setCurrentIndex(3);
      break;
    case 44100:
      d->alRate->setCurrentIndex(4);
      break;
    case 48000:
      d->alRate->setCurrentIndex(5);
      break;
    case 96000:
      d->alRate->setCurrentIndex(6);
      break;
    case 192000:
      d->alRate->setCurrentIndex(7);
      break;
    default:
      d->alRate->setCurrentIndex(0);
  }
  
  d->alMMap->setTristate();
  d->alSyncPtr->setTristate();
  d->alNonblock->setTristate();
  
  switch (alMmap_emulation)
  {
    case -1: d->alMMap->setCheckState(Qt::PartiallyChecked); break;
    case 0:  d->alMMap->setCheckState(Qt::Unchecked); break;
    case 1:  d->alMMap->setCheckState(Qt::Checked); break;
  }
  switch (alSync_ptr_ioctl)
  {
    case -1: d->alSyncPtr->setCheckState(Qt::PartiallyChecked); break;
    case 0:  d->alSyncPtr->setCheckState(Qt::Unchecked); break;
    case 1:  d->alSyncPtr->setCheckState(Qt::Checked); break;
  }
  switch (alNonblock)
  {
    case -1: d->alNonblock->setCheckState(Qt::PartiallyChecked); break;
    case 0:  d->alNonblock->setCheckState(Qt::Unchecked); break;
    case 1:  d->alNonblock->setCheckState(Qt::Checked); break;
  }
  
  if (d->exec()==QDialog::Rejected)
  {
    delete d;
    return;
  }
  
  emit componentChanged(this);
  
  if (d->alCard->currentIndex()!=0)
  {
    alCard=d->hwCnt[d->alCard->currentIndex()-1]->cardNum;
    if (d->alDevice->currentIndex()!=0)
    {
      alDevice=d->hwCnt[d->alCard->currentIndex()-1]->devices[d->alDevice->currentIndex()-1]->devNum;
      alSubdevice=d->alSubdevice->currentIndex()-1;
    } else
      alDevice=-1;
  } else
    alCard=-1;
  
  switch (d->alChannels->currentIndex())
  {
    case 1:
      alChannels=1;
      break;
    case 2:
      alChannels=2;
      break;
    case 3:
      alChannels=4;
      break;
    case 4:
      alChannels=6;
      break;
    case 5:
      alChannels=8;
      break;
    default:
      alChannels=-1;
  }
  
  switch (d->alRate->currentIndex())
  {
    case 1:
      alRate=8000;
      break;
    case 2:
      alRate=11025;
      break;
    case 3:
      alRate=22050;
      break;
    case 4:
      alRate=44100;
      break;
    case 5:
      alRate=48000;
      break;
    case 6:
      alRate=96000;
      break;
    case 7:
      alRate=192000;
      break;
    default:
      alRate=-1;
      break;
  }
  
  if (d->alFormat->currentIndex()>0)
    alFormat=d->alFormat->currentText().section(" ",0,0);
  else
    alFormat="<NONE>";
  
  switch (d->alMMap->checkState())
  {
    case Qt::Unchecked:        alMmap_emulation=0; break;
    case Qt::PartiallyChecked: alMmap_emulation=-1; break;
    case Qt::Checked:          alMmap_emulation=1; break;
  }
  switch (d->alSyncPtr->checkState())
  {
    case Qt::Unchecked:        alSync_ptr_ioctl=0; break;
    case Qt::PartiallyChecked: alSync_ptr_ioctl=-1; break;
    case Qt::Checked:          alSync_ptr_ioctl=1; break;
  }
  switch (d->alNonblock->checkState())
  {
    case Qt::Unchecked:        alNonblock=0; break;
    case Qt::PartiallyChecked: alNonblock=-1; break;
    case Qt::Checked:          alNonblock=1; break;
  }

  delete d;
  update();
}
