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

#ifndef QLADSPADLG_H
#define QLADSPADLG_H
#include <QtGui>
#include <QtCore>
#include "ui_qladspadlg.h"

enum TalControl {aacToggle,aacInteger,aacLinear,aacLogarithmic,aacFreq};

class QControlItem : public QObject
{
    Q_OBJECT
public:
    QControlItem(QWidget *parent, const QString &AportName, TalControl AaatType, bool AaasToggle, float AaasValue, QWidget* AaawControl, QLayout* AaawLayout, QLabel* AaawLabel);
    QControlItem(QControlItem*&);
    QControlItem();
    QControlItem(QDataStream &s);
    
    void storeToStream(QDataStream &s);
    void destroyControls();
    
    enum TalControl aatType;
    bool aasToggle;
    float aasValue;
    int aasFreq;
    int aasInt;
    QString portName;
    QWidget* aawControl;
    QLayout* aawLayout;
    QLabel* aawLabel;
};

QDataStream &operator<<(QDataStream &, const QControlItem* &);
QDataStream &operator>>(QDataStream &, QControlItem* &);

class QResizableFrame : public QFrame
{
    Q_OBJECT
public:
    QResizableFrame(QWidget *parent);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    QScrollArea* alScroller;
};

typedef QList<QControlItem*> QalCItems;

class QLADSPADialog : public QDialog, public Ui::QLADSPADialog
{
    Q_OBJECT

public:
    QLADSPADialog(QWidget *parent, int aSampleRate);
    ~QLADSPADialog();
    void setParams(QString &plugLabel, QString &plugID, QalCItems* aCItems);
    void getParams(QString &plugLabel, QString &plugID, QString &plugName, QString &plugFile, QalCItems* aCItems);
public slots:
    void changeLADSPA(int index);
    void valueChanged(double d);
    void stateChanged(int state);
private:
    QResizableFrame* alControls;
    QVBoxLayout* vboxCLayout;
    QStringList lsPluginFile;
    QStringList lsPluginName;
    QStringList lsPluginID;
    QStringList lsPluginLabel;
    QalCItems alCItems;
    QalCItems* psCItems;
    QString psPlugLabel;
    QString psPlugID;
    int selectedPlugin;
    int alSampleRate;
    bool isShowed;
    bool controlsRequested;
    void scanPlugins();
    void analysePlugin(int index);
    void clearCItems();
    void readInfoFromControls();
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent (QShowEvent * event);
};

#endif
