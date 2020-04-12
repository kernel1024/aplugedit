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
*   Parts of this code from ALSA project utilities:                       *
*   amixer, aplay, alsactl.                                               *
*                                                                         *
*   Copyright (c) by Takashi Iwai <tiwai@suse.de>                         *
*   Copyright (c) by Abramo Bagnara <abramo@alsa-project.org>             *
*   Copyright (c) by Jaroslav Kysela <perex@perex.cz>                     *
*   Based on vplay program by Michael Beck                                *
*                                                                         *
***************************************************************************/

#include "includes/alsabackend.h"
#include "includes/alsabackend_p.h"
#include "includes/generic.h"
#include <QDebug>

#define WARN(MSG) addAlsaWarning(QStringLiteral("%1: %2 (%3:%4)") \
    .arg(static_cast<const char *>(Q_FUNC_INFO)) \
    .arg(MSG) \
    .arg(static_cast<const char *>(__FILE__)) \
    .arg(__LINE__))

ZAlsaBackendPrivate::ZAlsaBackendPrivate(ZAlsaBackend *parent)
    : QObject(parent)
    , q_ptr(parent)
{
    m_mixerPollTimer.setInterval(1000);
    connect(&m_mixerPollTimer,&QTimer::timeout,this,&ZAlsaBackendPrivate::pollMixerEvents);
    m_mixerPollTimer.start();
}

ZAlsaBackendPrivate::~ZAlsaBackendPrivate()
{
    m_mixerPollTimer.stop();
    while (!m_mixerCtl.isEmpty()) {
        auto ctl = m_mixerCtl.takeLast();
        if (ctl != nullptr)
            snd_ctl_close(ctl);
    }
}

snd_ctl_t *ZAlsaBackendPrivate::getMixerCtl(int cardNum)
{
    if (cardNum>=m_mixerCtl.count()) {
        while (m_mixerCtl.count()<(cardNum+1))
            m_mixerCtl.append(nullptr);
    }

    snd_ctl_t* res = m_mixerCtl.at(cardNum);

    if (res == nullptr) {
        int err;

        QString name = QSL("hw:%1").arg(cardNum);
        if ((err = snd_ctl_open(&res, name.toLatin1().constData(), 0)) < 0) {
            WARN(QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err))));
            res = nullptr;
        }

        if ((err = snd_ctl_subscribe_events(res, 1)) < 0) {
            WARN(QSL("snd_ctl_subscribe_events: %1").arg(QString::fromUtf8(snd_strerror(err))));
            snd_ctl_close(res);
            res = nullptr;
        }
        m_mixerCtl[cardNum] = res;
    }

    return res;
}

void ZAlsaBackendPrivate::enumerateCards()
{
    m_cards.clear();

    static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;

    int card = -1;
    int err;
    int dev;

    snd_ctl_card_info_t *info = nullptr;
    snd_pcm_info_t *pcminfo = nullptr;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    if (snd_card_next(&card) < 0 || card < 0) {
        WARN(QSL("no soundcards found!"));
        return;
    }
    // **** List of Hardware Devices ****
    while (card >= 0) {
        snd_ctl_t *handle = getMixerCtl(card);
        if (handle) {
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                WARN(QSL("snd_ctl_card_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
            } else {

                m_cards << CCardItem(QString::fromUtf8(snd_ctl_card_info_get_id(info)),
                                     QString::fromUtf8(snd_ctl_card_info_get_name(info)),
                                     card);
                dev = -1;
                while (true) {
                    unsigned int count;
                    if (snd_ctl_pcm_next_device(handle, &dev)<0)
                        WARN(QSL("snd_ctl_pcm_next_device: %1").arg(QString::fromUtf8(snd_strerror(err))));
                    if (dev < 0)
                        break;
                    snd_pcm_info_set_device(pcminfo, static_cast<unsigned int>(dev));
                    snd_pcm_info_set_subdevice(pcminfo, 0);
                    snd_pcm_info_set_stream(pcminfo, stream);
                    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                            WARN(QSL("snd_ctl_pcm_info: %1").arg(QString::fromUtf8(snd_strerror(err))));
                        continue;
                    }
                    count = snd_pcm_info_get_subdevices_count(pcminfo);
                    m_cards.last().devices << CDeviceItem(dev,static_cast<int>(count),
                                                          QString::fromUtf8(snd_pcm_info_get_name(pcminfo)));
                }
            }
        }
        if (snd_card_next(&card) < 0)
            break;
    }
}

void ZAlsaBackendPrivate::snd_lib_error_handler(const char *file, int line, const char *function, int err, const char *fmt,...)
{
    va_list arg;
    va_start(arg, fmt);
    QString msg = QString::vasprintf(fmt,arg);
    if (err != 0)
        qWarning() << QSL(": %1").arg(QString::fromUtf8(snd_strerror(err)));
    va_end(arg);

    msg = QSL("ALSA: %1: %2 (%3:%4)")
            .arg(QString::fromUtf8(function),msg,QString::fromUtf8(file))
            .arg(line);

    auto alsa = gAlsa;
    if (alsa)
        Q_EMIT alsa->alsaWarningMsg(msg);
}

bool ZAlsaBackendPrivate::lessThanMixerItem(const CMixerItem &a, const CMixerItem &b)
{
    static const QStringList names({ QSL("Master"),
                                     QSL("Headphone"),
                                     QSL("Speaker"),
                                     QSL("Tone"),
                                     QSL("Bass"),
                                     QSL("Treble"),
                                     QSL("3D Control"),
                                     QSL("PCM"),
                                     QSL("Front"),
                                     QSL("Surround"),
                                     QSL("Center"),
                                     QSL("LFE"),
                                     QSL("Side"),
                                     QSL("Synth"),
                                     QSL("FM"),
                                     QSL("Wave"),
                                     QSL("Music"),
                                     QSL("DSP"),
                                     QSL("Line"),
                                     QSL("CD"),
                                     QSL("Mic"),
                                     QSL("Digital"),
                                     QSL("Video"),
                                     QSL("Zoom Video"),
                                     QSL("Phone"),
                                     QSL("I2S"),
                                     QSL("IEC958"),
                                     QSL("PC Speaker"),
                                     QSL("Beep"),
                                     QSL("Aux"),
                                     QSL("Mono"),
                                     QSL("Playback"),
                                     QSL("Capture"),
                                     QSL("Mix"),
                                   });

    static const QStringList subNames({ QSL("Mono"),
                                        QSL("Digital"),
                                        QSL("Switch"),
                                        QSL("Depth"),
                                        QSL("Wide"),
                                        QSL("Space"),
                                        QSL("Level"),
                                        QSL("Center"),
                                        QSL("Output"),
                                        QSL("Boost"),
                                        QSL("Tone"),
                                        QSL("Bass"),
                                        QSL("Treble"),
                                      });

    int weightA = names.count();
    int weightB = names.count();
    for (int i=0; i<names.count(); i++) {
        if ((weightA>i) && (a.name.startsWith(names.at(i))))
            weightA = i;
        if ((weightB>i) && (b.name.startsWith(names.at(i))))
            weightB = i;
    }
    if (weightA == weightB) {
        int weightA = subNames.count();
        int weightB = subNames.count();
        for (int i=0; i<subNames.count(); i++) {
            if ((weightA>i) && (a.name.endsWith(subNames.at(i))))
                weightA = i;
            if ((weightB>i) && (b.name.endsWith(subNames.at(i))))
                weightB = i;
        }
    }

    return weightA < weightB;
}

QVector<int> ZAlsaBackendPrivate::findRelatedMixerItems(const CMixerItem &base, const QVector<CMixerItem> &items, int* topScore)
{
    QVector<QPair<int,int> > scores;
    int maxScore = 0;
    for (int i=0; i<items.count(); i++) {
        if (base.numid == items.at(i).numid) continue;
        if (items.at(i).isRelated) continue;
        if (items.at(i).type != CMixerItem::itBoolean) continue;

        const QString a = base.name;
        const QString b = items.at(i).name;
        int maxLen = qMin(a.length(),b.length());
        int score = 0;
        while ((score<maxLen) && (a.at(score) == b.at(score)))
            score++;

        maxScore = qMax(maxScore,score);

        scores.append(qMakePair(i,score));
    }

    QVector<int> res;
    if (maxScore < base.name.length()/2) return res;
    if (topScore)
        *topScore = maxScore;

    for (const auto &pair : qAsConst(scores)) {
        if (pair.second == maxScore)
            res.append(pair.first);
    }

    return res;
}

void ZAlsaBackendPrivate::pollMixerEvents()
{
    Q_Q(ZAlsaBackend);

    const int ctlCount = m_mixerCtl.count();
    QScopedPointer<pollfd, QScopedPointerArrayDeleter<pollfd> > fds(new pollfd[ctlCount]);
    QVector<snd_ctl_t *> ctls;
    QHash<int,int> ctlIdx;

    for (int i=0; i< m_mixerCtl.count(); i++) {
        const auto ctl = m_mixerCtl.at(i);
        if (ctl) {
            snd_ctl_poll_descriptors(ctl, &fds.data()[ctls.count()], 1);
            ctlIdx[ctls.count()] = i;
            ctls.append(ctl);
        }
    }
    if (ctls.isEmpty()) return;

    int err = poll(fds.data(), ctls.count(), 0);
    if (err <= 0) return;

    for (int i = 0; i < ctls.count(); i++) {
        unsigned short revents;
        snd_ctl_poll_descriptors_revents(ctls.at(i), &fds.data()[i], 1, &revents);
        if (revents & POLLIN) {
            snd_ctl_event_t *event;
            snd_ctl_event_alloca(&event);

            if (snd_ctl_read(ctls.at(i), event) < 0) continue;
            if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM) continue;

            unsigned int mask = snd_ctl_event_elem_get_mask(event);

            if (mask == SND_CTL_EVENT_MASK_REMOVE) {
                Q_EMIT q->alsaMixerReconfigured(ctlIdx.value(i));
            } else {
                if (((mask & SND_CTL_EVENT_MASK_ADD) != 0) ||
                        ((mask & SND_CTL_EVENT_MASK_INFO) != 0)) {
                    Q_EMIT q->alsaMixerReconfigured(ctlIdx.value(i)); // reconfigure also load all values
                } else if (mask & SND_CTL_EVENT_MASK_VALUE) {
                    Q_EMIT q->alsaMixerValueChanged(ctlIdx.value(i));
                }
            }
        }
    }
}

void ZAlsaBackendPrivate::addAlsaWarning(const QString &msg)
{
    Q_Q(ZAlsaBackend);
    m_alsaWarnings.append(msg);
    Q_EMIT q->alsaWarningMsg(msg);
}
