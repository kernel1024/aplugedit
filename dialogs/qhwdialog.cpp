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


#include <alsa/asoundlib.h>
#include <QtGui>
#include <QtCore>
#include "includes/qhwdialog.h"

QHWDialog::QHWDialog(QWidget *parent)
  : QDialog(parent)
{
  setupUi(this);
  
  alCard->addItem("-not specified-");
  alCard->setCurrentIndex(0);
  
  // enumerating devices
  
  static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
  
  int card = -1;
  int err,dev;
  snd_ctl_t *handle;
      
  snd_ctl_card_info_t *info;
  snd_pcm_info_t *pcminfo;
  snd_ctl_card_info_alloca(&info);
  snd_pcm_info_alloca(&pcminfo);
  
  if (snd_card_next(&card) < 0 || card < 0) {
    printf("no soundcards found...");
    return;
  }
  // **** List of Hardware Devices ****
  while (card >= 0) {
    char name[32];
    sprintf(name, "hw:%d", card);
    
    if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
      printf("control open (%i): %s", card, snd_strerror(err));
      goto next_card;
    }
    if ((err = snd_ctl_card_info(handle, info)) < 0) {
      printf("control hardware info (%i): %s", card, snd_strerror(err));
      snd_ctl_close(handle);
      goto next_card;
    }
    
    hwCnt.append(new tlCards);
    hwCnt.last()->cardName=QString::fromUtf8(snd_ctl_card_info_get_name(info));
    hwCnt.last()->cardNum=card;
    dev = -1;
    while (1) {
      unsigned int count;
      if (snd_ctl_pcm_next_device(handle, &dev)<0)
        printf("snd_ctl_pcm_next_device");
      if (dev < 0)
        break;
      snd_pcm_info_set_device(pcminfo, dev);
      snd_pcm_info_set_subdevice(pcminfo, 0);
      snd_pcm_info_set_stream(pcminfo, stream);
      if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
        if (err != -ENOENT) printf("control digital audio info (%i): %s", card, snd_strerror(err));
        continue;
      }
      count = snd_pcm_info_get_subdevices_count(pcminfo);
      hwCnt.last()->devices.append(new tlDevices);
      hwCnt.last()->devices.last()->devNum=dev;
      hwCnt.last()->devices.last()->subdevices=count;
    }
    snd_ctl_close(handle);
  next_card:
    if (snd_card_next(&card) < 0) {
      break;
    }
  }
  
  for (int i=0;i<hwCnt.count();i++) alCard->addItem(hwCnt[i]->cardName);
  
  connect(alCard,SIGNAL(currentIndexChanged(int)),this,SLOT(cardSelected(int)));
  connect(alDevice,SIGNAL(currentIndexChanged(int)),this,SLOT(devSelected(int)));
  
  alCard->setCurrentIndex(0);
  cardSelected(0);
  
  alFormat->addItem("-not specified-");
  alFormat->addItem("S8 Signed 8 bit");
  alFormat->addItem("U8 Unsigned 8 bit");
  alFormat->addItem("S16_LE Signed 16 bit Little Endian");
  alFormat->addItem("S16_BE Signed 16 bit Big Endian");
  alFormat->addItem("U16_LE Unsigned 16 bit Little Endian");
  alFormat->addItem("U16_BE Unsigned 16 bit Big Endian");
  alFormat->addItem("S24_LE Signed 24 bit Little Endian");
  alFormat->addItem("S24_BE Signed 24 bit Big Endian");
  alFormat->addItem("U24_LE Unsigned 24 bit Little Endian");
  alFormat->addItem("U24_BE Unsigned 24 bit Big Endian");
  alFormat->addItem("S32_LE Signed 32 bit Little Endian");
  alFormat->addItem("S32_BE Signed 32 bit Big Endian");
  alFormat->addItem("U32_LE Unsigned 32 bit Little Endian");
  alFormat->addItem("U32_BE Unsigned 32 bit Big Endian");
  alFormat->addItem("FLOAT_LE Float 32 bit Little Endian, Range -1.0 to 1.0");
  alFormat->addItem("FLOAT_BE Float 32 bit Big Endian, Range -1.0 to 1.0");
  alFormat->addItem("FLOAT64_LE Float 64 bit Little Endian, Range -1.0 to 1.0");
  alFormat->addItem("FLOAT64_BE Float 64 bit Big Endian, Range -1.0 to 1.0");
  alFormat->addItem("IEC_958_SUBFRAME_LE IEC-958 Little Endian");
  alFormat->addItem("IEC_958_SUBFRAME_BE IEC-958 Big Endian");
  alFormat->addItem("MU_LAW u-Law format");
  alFormat->addItem("A_LAW a-Law format");
  alFormat->addItem("IMA_ADPCM Ima-ADPCM format");
  alFormat->addItem("MPEG compressed format");
  alFormat->addItem("GSM compressed format");
  alFormat->addItem("S24_3LE Signed 24bit Little Endian in 3bytes format");
  alFormat->addItem("S24_3BE Signed 24bit Big Endian in 3bytes format");
  alFormat->addItem("U24_3LE Unsigned 24bit Little Endian in 3bytes format");
  alFormat->addItem("U24_3BE Unsigned 24bit Big Endian in 3bytes format");
  alFormat->addItem("S20_3LE Signed 20bit Little Endian in 3bytes format");
  alFormat->addItem("S20_3BE Signed 20bit Big Endian in 3bytes format");
  alFormat->addItem("U20_3LE Unsigned 20bit Little Endian in 3bytes format");
  alFormat->addItem("U20_3BE Unsigned 20bit Big Endian in 3bytes format");
  alFormat->addItem("S18_3LE Signed 18bit Little Endian in 3bytes format");
  alFormat->addItem("S18_3BE Signed 18bit Big Endian in 3bytes format");
  alFormat->addItem("U18_3LE Unsigned 18bit Little Endian in 3bytes format");
  alFormat->addItem("U18_3BE Unsigned 18bit Big Endian in 3bytes format");

  alChannels->addItem("-not specified-");
  alChannels->addItem("1 - mono");
  alChannels->addItem("2 - stereo");
  alChannels->addItem("4 - quadro");
  alChannels->addItem("6 - 5.1 speakers");
  alChannels->addItem("8 - 7.1 speakers");
  
  alRate->addItem("-not specified-");
  alRate->addItem("8000 Hz");
  alRate->addItem("11025 Hz");
  alRate->addItem("22050 Hz");
  alRate->addItem("44,1 kHz");
  alRate->addItem("48 kHz");
  alRate->addItem("96 kHz");
  alRate->addItem("192 kHz");
  
  alMMap->setCheckState(Qt::PartiallyChecked);
  alSyncPtr->setCheckState(Qt::PartiallyChecked);
  alNonblock->setCheckState(Qt::PartiallyChecked);
}

QHWDialog::~QHWDialog()
{
  while (hwCnt.count()>0)
  {
    tlCards* c=hwCnt.takeLast();
    while (c->devices.count()>0)
    {
      tlDevices* d=c->devices.takeLast();
      delete d;
    }
    delete c;
  }
}

void QHWDialog::cardSelected(int index)
{
  alDevice->clear();
  alDevice->addItem("-not specified-");
  if (index<=0)
  {
    alDevice->setCurrentIndex(0);
    alDevice->setEnabled(false);
    return;
  }
  if (hwCnt.count()<=0) return;
  
  for (int i=0;i<hwCnt[index-1]->devices.count();i++)
    alDevice->addItem(QString::number(hwCnt[index-1]->devices[i]->devNum));
  alDevice->setCurrentIndex(0);
  
  alDevice->setEnabled(true);
}

void QHWDialog::devSelected(int index)
{
  alSubdevice->clear();
  alSubdevice->addItem("-not specified-");
  if (index<=0)
  {
    alSubdevice->setCurrentIndex(0);
    alSubdevice->setEnabled(false);
    return;
  }
  if (hwCnt.count()<=0) return;
  if (alDevice->currentIndex()<=0) return;
  
  for (int i=0;i<hwCnt[index-1]->devices[alDevice->currentIndex()-1]->subdevices;i++)
    alSubdevice->addItem(QString::number(i));
  alSubdevice->setCurrentIndex(0);
  
  alSubdevice->setEnabled(true);
}

