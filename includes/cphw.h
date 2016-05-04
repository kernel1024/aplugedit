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

#ifndef CPHW_H
#define CPHW_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class QCPHW : public QCPBase
{
  Q_OBJECT
public:
  void realignPins(QPainter & painter);
  void doInfoGenerate(QTextStream & stream);
  void showSettingsDlg();
  QCPInput* fInp;
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  
  int alCard, alDevice, alSubdevice;  // link to card, device number (default 0), subdevice number (default -1: first available)
  int alMmap_emulation, alSync_ptr_ioctl, alNonblock;  // enable mmap emulation for ro/wo devices, use SYNC_PTR ioctl rather than the direct mmap access for control structures, force non-blocking open mode
  QString alFormat; // restrict only to the given format
  int alChannels, alRate; // restrict only to the given channels, restrict only to the given rate
  
public:
  QCPHW(QWidget *parent, QRenderArea *aOwner);
  ~QCPHW();
  void readFromStream( QDataStream & stream );
  void storeToStream( QDataStream & stream );
protected:
  void paintEvent(QPaintEvent *event);
};
#endif
