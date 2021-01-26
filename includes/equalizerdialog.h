/***************************************************************************
*   Copyright (C) 2006 - 2021 by kernelonline@gmail.com                   *
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

#ifndef EQUALIZERDIALOG_H
#define EQUALIZERDIALOG_H

#include <QDialog>
#include "ladspadialog.h"

namespace Ui {
class ZEqualizerDialog;
}

class ZEqualizerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZEqualizerDialog(QWidget *parent = nullptr);
    ~ZEqualizerDialog() override;

    void setParams(int channels, const QString& controls, const CLADSPAPlugItem &plugin);
    void getParams(int &channels, QString& controls, CLADSPAPlugItem &plugin);

private:
    Ui::ZEqualizerDialog *ui;
    CLADSPAPlugItem m_plugin;

    void updatePluginLabel();
    void fixControls();

private Q_SLOTS:
    void browseControls();
    void selectPlugin();
    void resetPlugin();

};

#endif // EQUALIZERDIALOG_H
