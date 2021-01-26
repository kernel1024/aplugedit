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

#include <QDialog>
#include "ladspa_p.h"
#include "ui_ladspadlg.h"

class ZLADSPADialog : public QDialog, public Ui::ZLADSPADialog
{
    Q_OBJECT

public:
    ZLADSPADialog(QWidget *parent, int channels, int sampleRate);
    ~ZLADSPADialog() override;
    void setSimpleParamsMode(bool state);

    void setPlugItem(const CLADSPAPlugItem& item);
    CLADSPAPlugItem getPlugItem() const;

    int getChannels() const;

private Q_SLOTS:
    void comboPluginIndexChanged(int index);
    void valueChanged(double value);
    void stateChanged(int value);
    void addInputBinding();
    void deleteInputBinding();
    void addOutputBinding();
    void deleteOutputBinding();
    void policyChanged(bool status);

private:
    ZResizableFrame* m_controls;
    QVBoxLayout* m_vboxLayout;
    ZLADSPABindingsModel* m_inputsModel;
    ZLADSPABindingsModel* m_outputsModel;

    QHash<qint64,QString> m_pluginFile;
    QHash<qint64,QString> m_pluginName;
    QHash<qint64,QString> m_pluginLabel;
    QVector<CLADSPAControlItem> m_controlItems;

    QVector<CLADSPAControlItem> m_preservedControlItems;

    QStringList m_selectedPluginValidInputs;
    QStringList m_selectedPluginValidOutputs;

    qint64 m_selectedPluginID { 0L };
    qint64 m_preservedPlugID { 0 };
    int m_sampleRate { 44100 };
    int m_channels { 2 };
    bool m_isShowed { false };

    void scanPlugins();
    void analyzePlugin();
    void clearCItems();
    void readInfoFromControls();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent (QShowEvent *event) override;
};

#endif
