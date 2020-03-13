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

#ifndef LADSPALISTDIALOG_H
#define LADSPALISTDIALOG_H

#include <QtCore>
#include <QtWidgets>
#include "cpbase.h"
#include "ladspadialog.h"
#include "ladspa_p.h"

namespace Ui {
class ZLADSPAListDialog;
}

class ZLADSPAListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZLADSPAListDialog(QWidget *parent, int sampleRate);
    ~ZLADSPAListDialog();

    void setParams(int channels, const QVector<CLADSPAPlugItem> &plugins);
    void getParams(int &channels, QVector<CLADSPAPlugItem> &plugins);

private Q_SLOTS:
    void addPlugin();
    void deletePlugin();
    void editPlugin();

private:
    Ui::ZLADSPAListDialog *ui;
    ZLADSPAListModel *model;
    int m_sampleRate { 48000 };
};

#endif // LADSPALISTDIALOG_H
