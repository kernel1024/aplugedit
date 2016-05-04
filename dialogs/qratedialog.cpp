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


#include <QtGui>
#include <QtCore>
#include "includes/qratedialog.h"

QRateDialog::QRateDialog(QWidget *parent)
  : QDialog(parent)
{
  setupUi(this);
  
  alConverter->addItem("-not specified-");
  alConverter->addItem("samplerate best quality");
  alConverter->addItem("samplerate medium quality");
  alConverter->addItem("samplerate fastest");
  alConverter->addItem("samplerate order hold");
  alConverter->addItem("samplerate linear");
  
  alRate->addItem("8000 Hz");
  alRate->addItem("11025 Hz");
  alRate->addItem("22050 Hz");
  alRate->addItem("44,1 kHz");
  alRate->addItem("48 kHz");
  alRate->addItem("96 kHz");
  alRate->addItem("192 kHz");
  alRate->setCurrentIndex(4);
}

QRateDialog::~QRateDialog()
{
}

int QRateDialog::getRate()
{
  switch (alRate->currentIndex())
  {
    case 0: return 8000;
    case 1: return 11025;
    case 2: return 22050;
    case 3: return 44100;
    case 4: return 48000;
    case 5: return 96000;
    case 6: return 192000;
  }
  return 48000;
}

QString QRateDialog::getConverter()
{
  QString conv;
  switch (alConverter->currentIndex())
  {
    case 1: conv="samplerate_best"; break;
    case 2: conv="samplerate_medium"; break;
    case 3: conv="samplerate"; break;
    case 4: conv="samplerate_order"; break;
    case 5: conv="samplerate_linear"; break;
    default: conv="";
  }
  return conv;
}

void QRateDialog::setParams(int aRate, QString & aConverter)
{
  switch (aRate)
  {
    case 8000: alRate->setCurrentIndex(0); break;
    case 11025: alRate->setCurrentIndex(1); break;
    case 22050: alRate->setCurrentIndex(2); break;
    case 44100: alRate->setCurrentIndex(3); break;
    case 48000: alRate->setCurrentIndex(4); break;
    case 96000: alRate->setCurrentIndex(5); break;
    case 192000: alRate->setCurrentIndex(6); break;
    default: alRate->setCurrentIndex(4);
  }
  if (aConverter=="samplerate_best")
    alConverter->setCurrentIndex(1);
  else if (aConverter=="samplerate_medium")
    alConverter->setCurrentIndex(2);
  else if (aConverter=="samplerate")
    alConverter->setCurrentIndex(3);
  else if (aConverter=="samplerate_order")
    alConverter->setCurrentIndex(4);
  else if (aConverter=="samplerate_linear")
    alConverter->setCurrentIndex(5);
  else
    alConverter->setCurrentIndex(0);
}
