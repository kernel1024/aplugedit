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
***************************************************************************
*                                                                         *
*   Parts of this code from ALSA project aplay.c utility.                 *
*                                                                         *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#include <QApplication>
#include <alsa/asoundlib.h>
#include "includes/generic.h"
#include "includes/alsabackend.h"

ZAlsaBackend::ZAlsaBackend(QObject *parent) : QObject(parent)
{

}

ZAlsaBackend::~ZAlsaBackend() = default;

ZAlsaBackend *ZAlsaBackend::instance()
{
    static QPointer<ZAlsaBackend> inst;
    static QAtomicInteger<bool> initializedOnce(false);

    if (inst.isNull()) {
        if (initializedOnce.testAndSetAcquire(false,true)) {
            inst = new ZAlsaBackend(QApplication::instance());
            return inst.data();
        }

        qCritical() << "Accessing to gAlsa after destruction!!!";
        return nullptr;
    }

    return inst.data();
}

void ZAlsaBackend::initialize()
{
    enumerateCards();
}

void ZAlsaBackend::enumerateCards()
{
    m_cards.clear();

    static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    int card = -1;
    int err;
    int dev;
    snd_ctl_t *handle = nullptr;

    snd_ctl_card_info_t *info = nullptr;
    snd_pcm_info_t *pcminfo = nullptr;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    if (snd_card_next(&card) < 0 || card < 0) {
        qWarning() << "no soundcards found!";
        return;
    }
    // **** List of Hardware Devices ****
    while (card >= 0) {
        QString name = QSL("hw:%1").arg(card);

        if ((err = snd_ctl_open(&handle, name.toLatin1().constData(), 0)) < 0) {
            qWarning() << QSL("control open (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
            goto next_card;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0) {
            qWarning() << QSL("control hardware info (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
            snd_ctl_close(handle);
            goto next_card;
        }

        m_cards << CCardItem(QString::fromUtf8(snd_ctl_card_info_get_name(info)), card);
        dev = -1;
        while (true) {
            unsigned int count;
            if (snd_ctl_pcm_next_device(handle, &dev)<0)
                qWarning() << "snd_ctl_pcm_next_device";
            if (dev < 0)
                break;
            snd_pcm_info_set_device(pcminfo, static_cast<unsigned int>(dev));
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, stream);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                if (err != -ENOENT)
                    qWarning() << QSL("control digital audio info (%1): %2").arg(card).arg(QString::fromUtf8(snd_strerror(err)));
                continue;
            }
            count = snd_pcm_info_get_subdevices_count(pcminfo);
            m_cards.last().devices << CDeviceItem(dev,static_cast<int>(count),
                                                  QString::fromUtf8(snd_pcm_info_get_name(pcminfo)));
        }
        if (handle)
            snd_ctl_close(handle);
next_card:
        if (snd_card_next(&card) < 0) {
            break;
        }
    }
}

QList<CCardItem> ZAlsaBackend::cards() const
{
    return m_cards;
}

QList<CPCMItem> ZAlsaBackend::pcmList() const
{
    QList<CPCMItem> res;

    void **hints, **n;
    char *name, *descr, *io;
    const char *filter = "Output";

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return res;

    n = hints;
    while (*n != nullptr) {
        name = snd_device_name_get_hint(*n, "NAME");
        descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (!(io != nullptr && strcmp(io, filter) != 0)) {
            QStringList descList;
            if (descr != nullptr)
                descList = QString::fromUtf8(descr).split('\n');
            res.append(CPCMItem(QString::fromUtf8(name),descList));
        }
        if (name != nullptr)
            free(name);
        if (descr != nullptr)
            free(descr);
        if (io != nullptr)
            free(io);
        n++;
    }
    snd_device_name_free_hint(hints);

    return res;
}

CCardItem::CCardItem(const CCardItem &other)
{
    cardName = other.cardName;
    cardNum = other.cardNum;
    devices = other.devices;
}

CCardItem::CCardItem(const QString &aCardName, int aCardNum)
{
    cardName = aCardName;
    cardNum = aCardNum;
}

CDeviceItem::CDeviceItem(const CDeviceItem &other)
{
    devNum = other.devNum;
    subdevices = other.subdevices;
    devName = other.devName;
}

CDeviceItem::CDeviceItem(int aDevNum, int aSubdevices, const QString& aName)
{
    devNum = aDevNum;
    subdevices = aSubdevices;
    devName = aName;
}

CPCMItem::CPCMItem(const CPCMItem &other)
{
    name = other.name;
    description = other.description;
}

CPCMItem::CPCMItem(const QString &aName, const QStringList &aDescription)
{
    name = aName;
    description = aDescription;
}
