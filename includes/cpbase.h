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

#ifndef CPBASE_H
#define CPBASE_H 1

#include <QtCore>
#include <QtWidgets>

#define QPT_INPUT     1
#define QPT_OUTPUT    2
#define QCP_PINSIZE   8

class QCPInput;
class QCPOutput;
class QRenderArea;

class QCPBase : public QWidget
{
    Q_OBJECT

private:
    virtual void realignPins(QPainter & painter)=0;
    void mouseInPin(const QPoint & mx, int &aPinNum, int &aPinType, QCPBase * &aFilter);
    void checkRecycle();
    virtual void doInfoGenerate(QTextStream & stream)=0;
protected:
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void paintEvent(QPaintEvent *event);
    
public:
    QCPBase(QWidget *parent, QRenderArea *aOwner);
    
    virtual void readFromStream( QDataStream & stream );
    virtual void storeToStream( QDataStream & stream );
    void postLoadBind();
    
    void doGenerate(QTextStream & stream);
    virtual void showSettingsDlg();
    void redrawPins(QPainter & painter);
    virtual bool canConnectOut(QCPBase * toFilter);
    virtual bool canConnectIn(QCPBase * toFilter);

    QRenderArea *cpOwner;
    QList<QCPInput*> fInputs;
    QList<QCPOutput*> fOutputs;
    QPoint relCorner;
    bool isDragging, fSettingsDlg;
    QColor pinColor;
signals:
    void componentChanged(QCPBase * obj);
};

class QCPOutput : public QObject
{
    Q_OBJECT
public:
    QCPOutput(QObject * parent, QCPBase * aOwner);
    void readFromStream( QDataStream & stream );
    void storeToStream( QDataStream & stream );
    void postLoadBind();
    
    QCPBase *toFilter;
    QCPBase *ownerFilter;
    QPoint relCoord;
    qint32 toPin;
    QString pinName;
    QString ffLogic;
};

class QCPInput : public QObject
{
    Q_OBJECT
public:
    QCPInput(QObject * parent, QCPBase * aOwner);
    
    void readFromStream( QDataStream & stream );
    void storeToStream( QDataStream & stream );
    void postLoadBind();

    QCPBase * fromFilter;
    QCPBase * ownerFilter;
    QPoint relCoord;
    qint32 fromPin;
    QString pinName;
    QString ffLogic;
};

void debugPrint(const QString &s);
#endif
