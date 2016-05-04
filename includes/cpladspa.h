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

#ifndef CPLADSPA_H
#define CPLADSPA_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"
#include "qladspadialog.h"

class QCPLADSPA : public QCPBase
{
  Q_OBJECT
public:
  void realignPins(QPainter & painter);
  void doInfoGenerate(QTextStream & stream);
  QCPInput* fInp;
  QCPOutput* fOut;
  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  QCPLADSPA(QWidget *parent, QRenderArea *aOwner);
  ~QCPLADSPA();
protected:
  QString alPlugLabel;
  QString alPlugID;
  QString alPlugName;
  QString alPlugFile;
  QalCItems alCItems;
  void paintEvent ( QPaintEvent * event );
  void showSettingsDlg();
  int searchSampleRate();
  void readFromStream( QDataStream & stream );
  void storeToStream( QDataStream & stream );
};
#endif
