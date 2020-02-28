/***************************************************************************
*   Copyright (C) 2006 - 2020 by kernelonline@gmail.com                   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
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

class ZRenderArea : public QFrame
{
    Q_OBJECT
    friend class ZCPBase;
private:
    bool erroneousRoute { false };
    bool rectLinks { true };
    bool cbBuilding { false };
    int cbType { 0 };
    int cbPinNum { -1 };
    int cbConnCount { 0 };
    ZCPInput *cbInput { nullptr };
    ZCPOutput *cbOutput { nullptr };
    QScrollArea *scroller;
    QScopedPointer<QLabel,QScopedPointerDeleteLater> recycle;
    QPoint cbCurrent;
    QStringList nodeLocks;

public:
    explicit ZRenderArea(QScrollArea *aScroller = nullptr);
    
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool readSchematicLegacy(QDataStream& stream);
    bool readSchematic(const QByteArray& json);
    QByteArray storeSchematic() const;

    void repaintConn();
    void doGenerate(QTextStream& stream);

    void initConnBuilder(int aType, int aPinNum, ZCPInput* aInput, ZCPOutput* aOutput);
    void refreshConnBuilder(const QPoint& atPos);
    void doneConnBuilder(bool aNone, int aType, int aPinNum, ZCPInput* aInput, ZCPOutput* aOutput);
    bool postLoadBinding();
    void deleteComponents();
    int componentCount() const;

    ZCPBase* createCpInstance(const QString &className, const QPoint &pos = QPoint(),
                              const QString &objectName = QString());

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif
