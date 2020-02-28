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

#include <QtGui>
#include <QtCore>
#include "includes/ratedialog.h"
#include "includes/cpbase.h"

ZRateDialog::ZRateDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    alConverter->addItem(tr("-not specified-"));
    alConverter->addItem(tr("samplerate best quality"));
    alConverter->addItem(tr("samplerate medium quality"));
    alConverter->addItem(tr("samplerate fastest"));
    alConverter->addItem(tr("samplerate order hold"));
    alConverter->addItem(tr("samplerate linear"));

    alRate->addItem(tr("8000 Hz"));
    alRate->addItem(tr("11025 Hz"));
    alRate->addItem(tr("22050 Hz"));
    alRate->addItem(tr("44,1 kHz"));
    alRate->addItem(tr("48 kHz"));
    alRate->addItem(tr("96 kHz"));
    alRate->addItem(tr("192 kHz"));
    alRate->setCurrentIndex(4);
}

ZRateDialog::~ZRateDialog() = default;

int ZRateDialog::getRate()
{
    switch (alRate->currentIndex())
    {
        case 0: return 8000;
        case 1: return 11025;
        case 2: return 22050;
        case 3: return 44100;
        case 4: return 48000;
        case 5: return 96000;
        case 6: return 192000;
    }
    return 48000;
}

QString ZRateDialog::getConverter()
{
    QString conv;
    switch (alConverter->currentIndex())
    {
        case 1: conv=QSL("samplerate_best"); break;
        case 2: conv=QSL("samplerate_medium"); break;
        case 3: conv=QSL("samplerate"); break;
        case 4: conv=QSL("samplerate_order"); break;
        case 5: conv=QSL("samplerate_linear"); break;
        default: conv=QString();
    }
    return conv;
}

void ZRateDialog::setParams(int aRate, const QString &aConverter)
{
    switch (aRate)
    {
        case 8000: alRate->setCurrentIndex(0); break;
        case 11025: alRate->setCurrentIndex(1); break;
        case 22050: alRate->setCurrentIndex(2); break;
        case 44100: alRate->setCurrentIndex(3); break;
        case 48000: alRate->setCurrentIndex(4); break;
        case 96000: alRate->setCurrentIndex(5); break;
        case 192000: alRate->setCurrentIndex(6); break;
        default: alRate->setCurrentIndex(4);
    }
    if (aConverter==QSL("samplerate_best")) {
        alConverter->setCurrentIndex(1);
    } else if (aConverter==QSL("samplerate_medium")) {
        alConverter->setCurrentIndex(2);
    } else if (aConverter==QSL("samplerate")) {
        alConverter->setCurrentIndex(3);
    } else if (aConverter==QSL("samplerate_order")) {
        alConverter->setCurrentIndex(4);
    } else if (aConverter==QSL("samplerate_linear")) {
        alConverter->setCurrentIndex(5);
    } else {
        alConverter->setCurrentIndex(0);
    }
}
