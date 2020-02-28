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

#ifndef LADSPADLG_H
#define LADSPADLG_H
#include <QtWidgets>
#include <QtCore>
#include "ui_ladspadlg.h"

namespace ZLADSPA {
Q_NAMESPACE
enum Control {
    aacToggle,
    aacInteger,
    aacLinear,
    aacLogarithmic,
    aacFreq
};
Q_ENUM_NS(Control)
}

class ZLADSPAControlItem
{
public:

    ZLADSPAControlItem() = default;
    explicit ZLADSPAControlItem(QDataStream &s);
    explicit ZLADSPAControlItem(const QJsonValue &json);
    ZLADSPAControlItem(const ZLADSPAControlItem& other) = default;
    ZLADSPAControlItem(const QString &AportName, ZLADSPA::Control AaatType, bool AaasToggle,
                       double AaasValue, QWidget* AaawControl, QLayout* AaawLayout, QLabel* AaawLabel);
    
    QJsonValue storeToJson() const;
    void destroyControls();
    void disconnectFromControls();
    
    ZLADSPA::Control aatType { ZLADSPA::aacToggle };
    bool aasToggle { false };
    double aasValue { 0.0 };
    int aasFreq { 0 };
    int aasInt { 0 };
    QString portName;
    QWidget* aawControl { nullptr };
    QLayout* aawLayout { nullptr };
    QLabel* aawLabel { nullptr };
};

QDataStream &operator<<(QDataStream &, const ZLADSPAControlItem* &);
QDataStream &operator>>(QDataStream &, ZLADSPAControlItem* &);

class ZResizableFrame : public QFrame
{
    Q_OBJECT
public:
    ZResizableFrame(QWidget *parent);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    QScrollArea* alScroller;
};

using ZLADSPAControlItems = QList<ZLADSPAControlItem>;

class ZLADSPADialog : public QDialog, public Ui::ZLADSPADialog
{
    Q_OBJECT

public:
    ZLADSPADialog(QWidget *parent, int aSampleRate);
    ~ZLADSPADialog() override;
    void setParams(const QString &plugLabel, const QString &plugID, const ZLADSPAControlItems &controlItems);
    void getParams(QString &plugLabel, QString &plugID, QString &plugName, QString &plugFile, ZLADSPAControlItems &aCItems);
public Q_SLOTS:
    void changeLADSPA(int index);
    void valueChanged(double value);
    void stateChanged(int value);
private:
    ZResizableFrame* alControls;
    QVBoxLayout* vboxCLayout;
    QStringList lsPluginFile;
    QStringList lsPluginName;
    QStringList lsPluginID;
    QStringList lsPluginLabel;
    ZLADSPAControlItems alCItems;
    ZLADSPAControlItems m_preservedControlItems;
    QString m_preservedPlugLabel;
    QString m_preservedPlugID;
    int selectedPlugin { 0 };
    int alSampleRate { 48000 };
    bool isShowed { false };
    bool controlsRequested { false };
    void scanPlugins();
    void analyzePlugin(int index);
    void clearCItems();
    void readInfoFromControls();
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent (QShowEvent * event) override;
};

#endif
