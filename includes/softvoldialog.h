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

#ifndef SOFTVOLDIALOG_H
#define SOFTVOLDIALOG_H

#include <QDialog>

namespace Ui {
class ZSoftvolDialog;
}

class ZSoftvolDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::ZSoftvolDialog *ui;

public:
    explicit ZSoftvolDialog(QWidget *parent = nullptr);
    ~ZSoftvolDialog();

    void setParams(const QString& name, double min_dB, double max_dB, int resolution, int channels);
    void getParams(QString& name, double& min_dB, double& max_dB, int& resolution, int& channels);

private Q_SLOTS:
    void updateMaxDBLimits(double value);
    void updateResolution(bool state);
};

#endif // SOFTVOLDIALOG_H
