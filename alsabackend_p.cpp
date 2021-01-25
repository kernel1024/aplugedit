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
#include <chrono>

using namespace std::chrono_literals;

ZAlsaBackendPrivate::ZAlsaBackendPrivate(ZAlsaBackend *parent)
    : QObject(parent)
    , q_ptr(parent)
{
    qInstallMessageHandler(ZAlsaBackendPrivate::stdConsoleOutput);

    m_mixerPollTimer.setInterval(1s);
    connect(&m_mixerPollTimer,&QTimer::timeout,this,&ZAlsaBackendPrivate::pollMixerEvents);
    m_mixerPollTimer.start();
}

ZAlsaBackendPrivate::~ZAlsaBackendPrivate()
{
    m_mixerPollTimer.stop();
    closeAllMixerCtls();
}

void ZAlsaBackendPrivate::closeAllMixerCtls()
{
    for (const auto &ctl : qAsConst(m_mixerCtl)) {
        if (ctl.ctl != nullptr)
            snd_ctl_close(ctl.ctl);
    }
    m_mixerCtl.clear();
}

void ZAlsaBackendPrivate::enumerateMixers()
{
    closeAllMixerCtls();

    const QStringList nameBlacklist({ QSL("arcam_av"), QSL("sysdefault") });
    int err = 0;
    snd_ctl_t* ctl = nullptr;

    snd_ctl_card_info_t *info = nullptr;
    snd_ctl_card_info_alloca(&info);

    void **hints = nullptr;
    if ((err = snd_device_name_hint(-1, "ctl", &hints)) < 0) {
        qWarning() << QSL("snd_device_name_hint: %1").arg(QString::fromUtf8(snd_strerror(err)));
        return;
    }

    void **n = hints;
    QStringList loadedDispNames;

    while (*n != nullptr) {
        QScopedPointer<char,QScopedPointerPodDeleter> name(snd_device_name_get_hint(*n, "NAME"));
        QScopedPointer<char,QScopedPointerPodDeleter> descr(snd_device_name_get_hint(*n, "DESC"));
        QScopedPointer<char,QScopedPointerPodDeleter> io(snd_device_name_get_hint(*n, "IOID"));
        const QString name_s = QString::fromUtf8(name.data());

        if (!nameBlacklist.contains(name_s)) {
            QStringList descList;
            if (descr != nullptr)
                descList = QString::fromUtf8(descr.data()).split('\n');

            if ((err = snd_ctl_open(&ctl, name.data(), 0)) < 0) {
                qWarning() << QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err)));
                ctl = nullptr;
            }

            if (ctl) {
                QString displayName = name_s;
                if ((err = snd_ctl_card_info(ctl, info)) < 0) {
                    qWarning() << QSL("snd_ctl_card_info: %1").arg(QString::fromUtf8(snd_strerror(err)));
                } else {
                    displayName = QString::fromUtf8(snd_ctl_card_info_get_name(info));
                }

                if (!loadedDispNames.contains(displayName)) {
                    if ((err = snd_ctl_subscribe_events(ctl, 1)) < 0) {
                        qWarning() << QSL("snd_ctl_subscribe_events: %1").arg(QString::fromUtf8(snd_strerror(err)));
                        snd_ctl_close(ctl);
                        ctl = nullptr;
                    }
                    if (ctl) {
                        m_mixerCtl.append(CCTLItem(name_s,descList,displayName,ctl));
                        loadedDispNames.append(displayName);
                    }
                }
            }
        }
        n++;
    }
    snd_device_name_free_hint(hints);
}

snd_ctl_t *ZAlsaBackendPrivate::getMixerCtl(const QString& ctlName) const
{
    for (const auto &ctl : qAsConst(m_mixerCtl)) {
        if (ctl.name == ctlName)
            return ctl.ctl;
    }
    return nullptr;
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
        qWarning() << QSL("no soundcards found!");
        return;
    }
    // **** List of Hardware Devices ****
    while (card >= 0) {
        snd_ctl_t *handle = nullptr;
        const QString cardName = QSL("hw:%1").arg(card);
        if ((err = snd_ctl_open(&handle, cardName.toLatin1().constData(), 0)) < 0) {
            qWarning() << QSL("snd_ctl_open: %1").arg(QString::fromUtf8(snd_strerror(err)));
        } else {
            if (handle) {
                if ((err = snd_ctl_card_info(handle, info)) < 0) {
                    qWarning() << QSL("snd_ctl_card_info: %1").arg(QString::fromUtf8(snd_strerror(err)));
                } else {

                    m_cards << CCardItem(QString::fromUtf8(snd_ctl_card_info_get_id(info)),
                                         QString::fromUtf8(snd_ctl_card_info_get_name(info)),
                                         card);
                    dev = -1;
                    while (true) {
                        unsigned int count = 0;
                        if (snd_ctl_pcm_next_device(handle, &dev)<0)
                            qWarning() << QSL("snd_ctl_pcm_next_device: %1").arg(QString::fromUtf8(snd_strerror(err)));
                        if (dev < 0)
                            break;
                        snd_pcm_info_set_device(pcminfo, static_cast<unsigned int>(dev));
                        snd_pcm_info_set_subdevice(pcminfo, 0);
                        snd_pcm_info_set_stream(pcminfo, stream);
                        if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                            if (err != -ENOENT)
                                qWarning() << QSL("snd_ctl_pcm_info: %1").arg(QString::fromUtf8(snd_strerror(err)));
                            continue;
                        }
                        count = snd_pcm_info_get_subdevices_count(pcminfo);
                        m_cards.last().devices << CDeviceItem(dev,static_cast<int>(count),
                                                              QString::fromUtf8(snd_pcm_info_get_name(pcminfo)));
                    }
                }
                snd_ctl_close(handle);
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

    auto *alsa = gAlsa;
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

QVector<int> ZAlsaBackendPrivate::findRelatedMixerItems(const CMixerItem &base, const QVector<CMixerItem> &items,
                                                        int* topScore) const
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

    QScopedPointer<pollfd, QScopedPointerArrayDeleter<pollfd> >
            fds(new pollfd[static_cast<unsigned int>(m_mixerCtl.count())]);

    for (int i = 0; i < m_mixerCtl.count(); i++)
        snd_ctl_poll_descriptors(m_mixerCtl.at(i).ctl, &fds.data()[i], 1);

    int err = poll(fds.data(), static_cast<nfds_t>(m_mixerCtl.count()), 0);
    if (err <= 0) return;

    for (int i = 0; i < m_mixerCtl.count(); i++) {
        unsigned short revents = 0;
        snd_ctl_poll_descriptors_revents(m_mixerCtl.at(i).ctl, &fds.data()[i], 1, &revents);
        if ((revents & POLLIN) != 0) {
            snd_ctl_event_t *event = nullptr;
            snd_ctl_event_alloca(&event);

            if (snd_ctl_read(m_mixerCtl.at(i).ctl, event) < 0) continue;
            if (snd_ctl_event_get_type(event) != SND_CTL_EVENT_ELEM) continue;

            unsigned int mask = snd_ctl_event_elem_get_mask(event);

            if (mask == SND_CTL_EVENT_MASK_REMOVE) {
                Q_EMIT q->alsaMixerReconfigured(m_mixerCtl.at(i).name);
            } else {
                if (((mask & SND_CTL_EVENT_MASK_ADD) != 0) ||
                        ((mask & SND_CTL_EVENT_MASK_INFO) != 0)) {
                    Q_EMIT q->alsaMixerReconfigured(m_mixerCtl.at(i).name); // reconfigure also load all values
                } else if ((mask & SND_CTL_EVENT_MASK_VALUE) != 0) {
                    Q_EMIT q->alsaMixerValueChanged(m_mixerCtl.at(i).name);
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

void ZAlsaBackendPrivate::stdConsoleOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString lmsg = QString();
    int line = context.line;
    QString file(QString::fromUtf8(context.file));
    QString category(QString::fromUtf8(context.category));
    if (category==QSL("default")) {
        category.clear();
    } else {
        category.append(' ');
    }

    switch (type) {
        case QtDebugMsg:
            lmsg = QSL("%1DEBUG: %2 (%3:%4)").arg(category, msg, file, QString::number(line));
            break;
        case QtWarningMsg:
            lmsg = QSL("%1WARN: %2 (%3:%4)").arg(category, msg, file, QString::number(line));
            break;
        case QtCriticalMsg:
            lmsg = QSL("%1ERROR: %2 (%3:%4)").arg(category, msg, file, QString::number(line));
            break;
        case QtFatalMsg:
            lmsg = QSL("%1FATAL: %2 (%3:%4)").arg(category, msg, file, QString::number(line));
            break;
        case QtInfoMsg:
            lmsg = QSL("%1INFO: %2 (%3:%4)").arg(category, msg, file, QString::number(line));
            break;
    }

    auto *alsa = gAlsa;
    if (!lmsg.isEmpty() && (alsa != nullptr)) {
        alsa->d_func()->addDebugOutputPrivate(lmsg);
        QString fmsg = QSL("%1 %2\n").arg(QTime::currentTime()
                                                   .toString(QSL("h:mm:ss")),lmsg);
        if (ZGenericFuncs::runnedFromQtCreator()) {
            fprintf(stderr, "%s", fmsg.toLocal8Bit().constData());
        }
    }
}

void ZAlsaBackendPrivate::addDebugOutputPrivate(const QString &msg)
{
    Q_Q(ZAlsaBackend);
    QMutexLocker lock(&m_loggerMutex);
    const int maxMessagesCount = 5000;

    m_debugMessages.append(msg);
    while (m_debugMessages.count()>maxMessagesCount)
        m_debugMessages.removeFirst();

    Q_EMIT q->debugOutputUpdated();
}
