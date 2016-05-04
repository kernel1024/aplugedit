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

#ifndef RENDERAREA_H
#define RENDERAREA_H 1

#include <QtCore>
#include <QtGui>
#include "cpbase.h"

class QRenderArea : public QFrame
{
    Q_OBJECT
public:
    QRenderArea(QWidget *parent = 0, QScrollArea *aScroller=0);
    
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    
    QScrollArea *scroller;
    
    bool resReading, erroneousRoute, rectLinks;
    QStringList nodeLocks;
    QLabel* recycle;
    QDataStream* storeStream;
    
    int cbType, cbPinNum, cbConnCount;
    QCPInput *cbInput;
    QCPOutput *cbOutput;
    bool cbBuilding;
    QPoint cbCurrent;
    
    void initConnBuilder(const int aType, int aPinNum, QCPInput* aInput, QCPOutput* aOutput);
    void repaintConn();
    void refreshConnBuilder(const QPoint & atPos);
    void doneConnBuilder(const bool aNone, int aType, const int aPinNum, QCPInput* aInput, QCPOutput* aOutput);
    void postLoadBinding();
    void readSchematic(QDataStream & stream);
    void storeSchematic(QDataStream & stream);
    void deleteComponents();
    void doGenerate(QTextStream & stream);
    int cpComponentCount();
protected:
    void paintEvent ( QPaintEvent * event );
    QCPBase* createCpInstance(QString & className);
};

#endif
