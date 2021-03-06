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

#include <chrono>
#include <QtWidgets>
#include "includes/generic.h"
#include "includes/generatedialog.h"

using namespace std::chrono_literals;

ZGenerateDialog::ZGenerateDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(saveButton,&QPushButton::clicked,this,&ZGenerateDialog::saveAs);
    connect(fontButton,&QPushButton::clicked,this,&ZGenerateDialog::fontDlg);

    QSettings stg;
    stg.beginGroup(QSL("generateDialog"));
    QFont editorFont;
    if (editorFont.fromString(stg.value(QSL("editorFont"),QSL("Monospace")).toString()))
        configEditor->setFont(editorFont);
    stg.endGroup();
}

void ZGenerateDialog::setConfigText(const QString &text)
{
    configEditor->setPlainText(text);
}

void ZGenerateDialog::setWarnings(const QStringList &warnings)
{
    m_warnings = warnings;
}

void ZGenerateDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    if (!m_warnings.isEmpty()) {
        QStringList warnings = m_warnings;
        QTimer::singleShot(1s,this,[this,warnings](){
            ZGenericFuncs::showWarningsDialog(this,tr("ALSA config problems"),
                                                  tr("Config was generated with %1 warnings.").arg(warnings.count()),
                                                  warnings);
        });
        m_warnings.clear();
    }
}

ZGenerateDialog::~ZGenerateDialog()
{
    QSettings stg;
    stg.beginGroup(QSL("generateDialog"));
    stg.setValue(QSL("editorFont"),configEditor->font().toString());
    stg.endGroup();
}

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

void ZGenerateDialog::fontDlg()
{
    bool ok = 0;
    QFont font = QFontDialog::getFont(&ok,configEditor->font(),this,tr("Editor font"),QFontDialog::MonospacedFonts);
    if (ok)
        configEditor->setFont(font);
}

