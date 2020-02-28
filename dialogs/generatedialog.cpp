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

#include <QtWidgets>
#include <QtCore>
#include "includes/generatedialog.h"

ZGenerateDialog::ZGenerateDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(saveButton,&QPushButton::clicked,this,&ZGenerateDialog::saveAs);
}

void ZGenerateDialog::setConfigText(const QString &text)
{
    configEditor->setPlainText(text);
}

ZGenerateDialog::~ZGenerateDialog() = default;

void ZGenerateDialog::saveAs()
{
    QString fname = QFileDialog::getSaveFileName(this,tr("Choose a filename to save config under"));
    if (fname.isEmpty()) return;

    QFile file(fname);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this,tr("File error"),tr("Cannot create file %1").arg(fname));
        return;
    }
    file.write(configEditor->toPlainText().toUtf8());
    file.close();
}

