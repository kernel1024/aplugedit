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
#include "alsabackend.h"

class ZRenderArea : public QFrame
{
    Q_OBJECT
    friend class ZCPBase;
private:
    bool m_connBuilding { false };
    int m_startPinType { 0 };
    int m_startPinNum { -1 };
    int m_connCount { 0 };
    ZCPInput *m_startInput { nullptr };
    ZCPOutput *m_startOutput { nullptr };
    ZCPBase *m_startFilter { nullptr };
    QScrollArea *m_scroller { nullptr };
    QLabel *m_recycle { nullptr };
    QPoint m_connCursor;

    void drawArrowLine(QPainter *p, const QPoint& p1, const QPoint& p2,
                       bool invertDirection = false, bool arrowAtEnd = false);
    void paintConnections(QPainter *p);
    void initConnBuilder(int type, int pinNum, ZCPInput* input, ZCPOutput* output,
                         ZCPBase* initialFilter);
    void refreshConnBuilder(const QPoint& atPos);
    void doneConnBuilder(bool none, int type, int pinNum, ZCPInput* input, ZCPOutput* output,
                         ZCPBase* finishFilter);
    bool postLoadBinding();

public:
    explicit ZRenderArea(QScrollArea *aScroller = nullptr);
    
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool readSchematicLegacy(QDataStream& stream);
    bool readSchematic(const QByteArray& json);
    QByteArray storeSchematic() const;

    void repaintConn();
    void doGenerate(QTextStream& stream, QStringList& warnings);

    QVector<CPCMItem> getAllPCMNames() const;

    void deleteComponents();
    int componentCount() const;

    ZCPBase* createCpInstance(const QString &className, const QPoint &pos = QPoint(),
                              const QString &objectName = QString());

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif
