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

#ifdef WITH_GST

#include <QtCore>
#include <QtWidgets>
#include "includes/generic.h"
#include "includes/sampleplayer.h"
#include "includes/alsabackend.h"
#include "ui_sampleplayer.h"
#include <QDebug>

ZSamplePlayer::ZSamplePlayer(QWidget *parent, ZRenderArea *renderArea) :
    QDialog(parent),
    ui(new Ui::ZSamplePlayer),
    m_renderArea(renderArea)
{
    ui->setupUi(this);

    m_syntax = new ZSpecLogHighlighter(ui->editLog->document());

    connect(ui->buttonPlay,&QPushButton::clicked,this,&ZSamplePlayer::play);
    connect(ui->buttonStop,&QPushButton::clicked,this,&ZSamplePlayer::stop);
    connect(ui->buttonBrowse,&QPushButton::clicked,this,&ZSamplePlayer::browseFile);
    connect(ui->editLog, &QTextEdit::textChanged,this,[this](){
        ui->linesCount->setText(tr("%1 messages").arg(ui->editLog->document()->lineCount() - 1));
    });
    connect(this,&ZSamplePlayer::stopped,ui->levelL,&ZLevelMeter::reset);
    connect(this,&ZSamplePlayer::stopped,ui->levelR,&ZLevelMeter::reset);
    connect(ui->sliderVolume,&QSlider::valueChanged,this,&ZSamplePlayer::updateVolume);

    QSettings stg;
    stg.beginGroup(QSL("SamplePlayer"));
    ui->editFilename->setText(stg.value(QSL("sampleFile"),QString()).toString());
    ui->comboWaveform->setCurrentText(stg.value(QSL("generatorWaveform"),QSL("sine")).toString());
    ui->spinFrequency->setValue(stg.value(QSL("generatorFreq"),440.0).toDouble());
    ui->sliderVolume->setValue(stg.value(QSL("volume"),90).toInt());

    ui->radioFuncGenerator->setChecked(true); // force widged disabling logic
    ui->radioMediaFile->setChecked(!(stg.value(QSL("funcGenerator"),false).toBool()));
    stg.endGroup();

    ui->comboAlsaSink->setItemDelegate(new ZDescListItemDelegate());

    ui->labelClock->clear();
    m_positionTimer = new QTimer(this);
    connect(m_positionTimer,&QTimer::timeout,this,&ZSamplePlayer::updatePosition);
    m_positionTimer->setInterval(500);
    m_positionTimer->start();

    updateSinkList();
    initGstreamer();
}

ZSamplePlayer::~ZSamplePlayer()
{
    stop();

    QSettings stg;
    stg.beginGroup(QSL("SamplePlayer"));
    stg.setValue(QSL("sampleFile"),ui->editFilename->text());
    stg.setValue(QSL("generatorWaveform"),ui->comboWaveform->currentText());
    stg.setValue(QSL("generatorFreq"),ui->spinFrequency->value());
    stg.setValue(QSL("funcGenerator"),ui->radioFuncGenerator->isChecked());
    stg.setValue(QSL("volume"),ui->sliderVolume->value());
    stg.endGroup();

    delete ui;
}

gboolean ZSamplePlayer::bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    Q_UNUSED(bus)
    auto dlg = qobject_cast<ZSamplePlayer *>(reinterpret_cast<QObject *>(data));

    gchar  *debug = nullptr;
    GError *error = nullptr;
    QString message;
    QString prefix = QSL("AUX");

    switch (GST_MESSAGE_TYPE(msg)) {

        case GST_MESSAGE_EOS:
            if (dlg) {
                QTimer::singleShot(0,dlg,[dlg](){
                    dlg->addAuxMessage(tr("APlugEdit: End of stream"));
                    dlg->stop();
                });
            }
            break;
        case GST_MESSAGE_ERROR:   gst_message_parse_error(msg, &error, &debug); prefix = QSL("ERROR"); break;
        case GST_MESSAGE_WARNING: gst_message_parse_warning(msg,&error,&debug);  prefix = QSL("WARN"); break;
        case GST_MESSAGE_INFO:    gst_message_parse_info(msg, &error, &debug); prefix = QSL("INFO"); break;
        case GST_MESSAGE_STATE_CHANGED: {
            if (dlg &&
                    (GST_MESSAGE_SRC(msg) == GST_OBJECT(dlg->m_data.pipeline))) {
                GstState olds;
                GstState news;
                GstState pending;
                gst_message_parse_state_changed(msg, &olds, &news, &pending);
                message = QSL("STATE: Pipeline: %1 -> %2")
                          .arg(QString::fromUtf8(gst_element_state_get_name(olds)),
                               QString::fromUtf8(gst_element_state_get_name(news)));

                if (news==GST_STATE_PLAYING)
                    QMetaObject::invokeMethod(dlg,&ZSamplePlayer::getSinkInfo);
            }
            break;
        }
        case GST_MESSAGE_ELEMENT: {
            const GstStructure *s = gst_message_get_structure(msg);
            const gchar *name = gst_structure_get_name(s);

            if (strcmp(name, "level") == 0) {
                guint channels;
                gdouble rms_dB;
                gdouble peak_dB;
                gdouble rmsL;
                gdouble rmsR;
                gdouble peakL;
                gdouble peakR;
                const GValue *array_val;
                const GValue *valueRms;
                const GValue *valuePeak;
                GValueArray *rms_arr;
                GValueArray *peak_arr;

                /* the values are packed into GValueArrays with the value per channel */
                array_val = gst_structure_get_value (s, "rms");
                rms_arr = static_cast<GValueArray *>(g_value_get_boxed (array_val));

                array_val = gst_structure_get_value (s, "peak");
                peak_arr = static_cast<GValueArray *>(g_value_get_boxed (array_val));

                /* we can get the number of channels as the length of any of the value
                   * arrays */
                channels = rms_arr->n_values;
                if (channels>0) {
                    valueRms = rms_arr->values;
                    rms_dB = g_value_get_double(valueRms);
                    valuePeak = peak_arr->values;
                    peak_dB = g_value_get_double(valuePeak);
                    rmsL = pow(10, rms_dB / 20);
                    peakL = pow(10, peak_dB / 20);
                    if (channels>1) {
                        rms_dB = g_value_get_double(++valueRms);
                        peak_dB = g_value_get_double(++valuePeak);
                        rmsR = pow(10, rms_dB / 20);
                        peakR = pow(10, peak_dB / 20);
                    } else {
                        rmsR = rmsL;
                        peakR = peakL;
                    }
                    QMetaObject::invokeMethod(dlg,[rmsL,rmsR,peakL,peakR,dlg](){
                        dlg->updateLevelIndicator(rmsL,rmsR,peakL,peakR);
                    });
                }
            }
            break;
        }
        default:
            break;
    }

    if (debug)
        g_free(debug);
    if (error) {
        message = QSL("%1: Pipeline: %2").arg(prefix,QString::fromUtf8(error->message));
        g_error_free (error);
    }

    if (dlg && !message.isEmpty())
        ZSamplePlayer::addAuxMessageExt(dlg,message);

    return TRUE;
}

void ZSamplePlayer::pad_added_handler(GstElement *src, GstPad *new_pad, gpointer *data)
{
    auto dlg = qobject_cast<ZSamplePlayer *>(reinterpret_cast<QObject *>(data));
    if (dlg==nullptr) return;

    GstPad *sink_pad = gst_element_get_static_pad(dlg->m_data.audioconvert, "sink");
    GstPadLinkReturn ret;
    GstCaps *new_pad_caps = nullptr;
    GstStructure *new_pad_struct = nullptr;
    const gchar *new_pad_type = nullptr;

    ZSamplePlayer::addAuxMessageExt(dlg,QSL("APlugEdit: Received new pad '%1' from '%2'")
                                    .arg(QString::fromUtf8(GST_PAD_NAME(new_pad)),
                                         QString::fromUtf8(GST_ELEMENT_NAME(src))));

    /* If our converter is already linked, we have nothing to do here */
    if (gst_pad_is_linked(sink_pad) == FALSE) {

        /* Check the new pad's type */
        new_pad_caps = gst_pad_get_current_caps(new_pad);
        new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
        new_pad_type = gst_structure_get_name(new_pad_struct);
        if (g_str_has_prefix(new_pad_type, "audio/x-raw") == FALSE) {
            ZSamplePlayer::addAuxMessageExt(dlg,QSL("APlugEdit: Decoder pad has type '%1' which is not raw audio. Ignoring.")
                                            .arg(QString::fromUtf8(new_pad_type)));
        } else {

            /* Attempt the link */
            ret = gst_pad_link(new_pad, sink_pad);
            if (GST_PAD_LINK_FAILED (ret)) {
                ZSamplePlayer::addAuxMessageExt(dlg,QSL("APlugEdit: Decoder pad type is '%1' but link failed.")
                                                .arg(QString::fromUtf8(new_pad_type)));
            } else {
                ZSamplePlayer::addAuxMessageExt(dlg,QSL("APlugEdit: Link succeeded (decoder pad type '%1').")
                                                .arg(QString::fromUtf8(new_pad_type)));
            }
        }
    }

    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != nullptr)
        gst_caps_unref(new_pad_caps);

    /* Unreference the sink pad */
    gst_object_unref(sink_pad);
}

void ZSamplePlayer::debug_logger(GstDebugCategory *category, GstDebugLevel level, const gchar *file,
                                 const gchar *function, gint line, GObject *object, GstDebugMessage *message,
                                 gpointer user_data)
{
    Q_UNUSED(object)

    auto dlg = qobject_cast<ZSamplePlayer *>(reinterpret_cast<QObject *>(user_data));
    if (dlg==nullptr) return;

    QString slevel;
    switch (level) {
        case GST_LEVEL_ERROR: slevel = QSL("ERROR"); break;
        case GST_LEVEL_WARNING: slevel = QSL("WARN"); break;
        case GST_LEVEL_FIXME: slevel = QSL("FIXME"); break;
        case GST_LEVEL_INFO: slevel = QSL("INFO"); break;
        case GST_LEVEL_DEBUG: slevel = QSL("DEBUG"); break;
        case GST_LEVEL_LOG: slevel = QSL("LOG"); break;
        case GST_LEVEL_TRACE: slevel = QSL("TRACE"); break;
        case GST_LEVEL_MEMDUMP: slevel = QSL("MEMDUMP"); break;
        default: slevel = QSL("MSG");
    }

    QString msg = QSL("%1: %2: %3: %4 (%5:%6)")
                  .arg(slevel,
                       QString::fromUtf8(gst_debug_category_get_name(category)),
                       QString::fromUtf8(function),
                       QString::fromUtf8(gst_debug_message_get(message)),
                       QString::fromUtf8(file))
                  .arg(line);

    ZSamplePlayer::addAuxMessageExt(dlg,msg);
}

void ZSamplePlayer::initGstreamer()
{
    static bool gstInitialized = false;
    if (gstInitialized) return;

    /* Initialization */
    gst_init(nullptr, nullptr);

    /* Debug level settings */
    if (gst_debug_is_active() == FALSE) {
        gst_debug_set_active(TRUE);
        GstDebugLevel dbglevel = gst_debug_get_default_threshold();
        if (dbglevel < GST_LEVEL_FIXME) {
            dbglevel = GST_LEVEL_FIXME;
            gst_debug_set_default_threshold(dbglevel);
            //gst_debug_set_threshold_from_string("3,GST_REFCOUNTING:5",TRUE);
        }
    }

    gstInitialized = true;
}

bool ZSamplePlayer::startGstreamer()
{
    if (m_data.pipeline) {
        stop();
    }

    gst_debug_add_log_function(debug_logger,this,nullptr);
    gst_debug_remove_log_function(gst_debug_log_default);

    bool useGenerator = ui->radioFuncGenerator->isChecked();

    /* Create gstreamer elements */
    m_data.pipeline = gst_pipeline_new("pipeline");
    if (useGenerator) {
        m_data.source = gst_element_factory_make("audiotestsrc", "source");
    } else {
        m_data.source = gst_element_factory_make("uridecodebin", "source");
    }
    m_data.audioconvert = gst_element_factory_make("audioconvert", "convert");
    m_data.audioresample = gst_element_factory_make("audioresample", "resample");
    m_data.meter = gst_element_factory_make("level", "meter");
    m_data.volume = gst_element_factory_make("volume", nullptr);
    m_data.alsasink = gst_element_factory_make("alsasink", "sink");

    if (!m_data.pipeline || !m_data.source || !m_data.audioconvert
            || !m_data.audioresample || !m_data.volume || !m_data.alsasink) {
        addAuxMessage(tr("APlugEdit: one of pipeline element could not be created."));
        return false;
    }

    /* Set up the pipeline */

    /* we add all elements into the pipeline */
    gst_bin_add_many(GST_BIN(m_data.pipeline),m_data.source,m_data.audioconvert,
                     m_data.audioresample,m_data.volume,m_data.alsasink,nullptr);
    if (m_data.meter)
        gst_bin_add(GST_BIN(m_data.pipeline),m_data.meter);

    gboolean lnkres = FALSE;
    if (m_data.meter) {
        lnkres = gst_element_link_many(m_data.audioconvert, m_data.audioresample,m_data.meter,
                                       m_data.volume,m_data.alsasink, nullptr);
    } else {
        lnkres = gst_element_link_many(m_data.audioconvert, m_data.audioresample,
                                       m_data.volume,m_data.alsasink, nullptr);
    }

    if (lnkres == FALSE) {
        addAuxMessage(tr("APlugEdit: Unable to link pipeline."));
        stop();
        return false;
    }

    if (useGenerator) {
        /* link generator to pipeline without dynamic pad allocation */
        if (gst_element_link(m_data.source, m_data.audioconvert) == FALSE) {
            addAuxMessage(tr("APlugEdit: Unable to link function generator source."));
            stop();
            return false;
        }

        /* set generator params */
        gdouble freq = ui->spinFrequency->value();
        g_object_set(m_data.source, "freq", freq, nullptr);

        g_object_set(m_data.source, "wave", ui->comboWaveform->currentIndex(), nullptr);

    } else {
        /* set mediafile uri */
        QUrl uri = QUrl::fromUserInput(ui->editFilename->text());
        QByteArray buri = uri.toEncoded();
        g_object_set(m_data.source, "uri", buri.constData(),nullptr);

        /* Connect to the pad-added signal */
        g_signal_connect(m_data.source, "pad-added", G_CALLBACK(pad_added_handler), this);
    }

    /* make sure we'll get messages from meter */
    if (m_data.meter)
        g_object_set (m_data.meter, "post-messages", TRUE, NULL);

    QByteArray bsink = ui->comboAlsaSink->currentText().toLatin1();
    if (!bsink.isEmpty())
        g_object_set(m_data.alsasink,"device", bsink.constData(), nullptr);

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_data.pipeline));
    m_data.busWatchID = gst_bus_add_watch(bus, bus_call, this);
    gst_object_unref(bus);

    m_data.positionQuery = gst_query_new_position(GST_FORMAT_TIME);

    updateVolume(ui->sliderVolume->value());

    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state(m_data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        addAuxMessage(tr("APlugEdit: Unable to set the pipeline to the playing state."));
        stop();
        return false;
    }

    return true;
}

void ZSamplePlayer::play()
{
    startGstreamer();
}

void ZSamplePlayer::stop()
{
    if (m_data.pipeline) {
        gst_element_set_state(m_data.pipeline, GST_STATE_NULL);

        gst_object_unref(GST_OBJECT(m_data.positionQuery));
        gst_object_unref(GST_OBJECT(m_data.pipeline));

        if (m_data.busWatchID>0)
            g_source_remove(m_data.busWatchID);

        m_data.clear();

        gst_debug_add_log_function(gst_debug_log_default,stderr,nullptr);
        gst_debug_remove_log_function(debug_logger);

        Q_EMIT stopped();
    }
}

void ZSamplePlayer::browseFile()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open sample file"),QString(),
                                                    tr("Media files [*.wav *.mp3 *.ogg *.flac] "
                                                       "(*.wav *.mp3 *.ogg *.flac);;"
                                                       "All files (*)"));
    if (!filename.isEmpty())
        ui->editFilename->setText(filename);

}

void ZSamplePlayer::addAuxMessageExt(ZSamplePlayer *instance, const QString &msg)
{
    // Threadsafe message adder for callbacks
    if (instance==nullptr) {
        qCritical() << QSL("Unable to add message to null-instance \"%1\"").arg(msg);
        return;
    }
    QTimer::singleShot(0,instance,[instance,msg](){
        instance->addAuxMessage(msg);
    });
}

void ZSamplePlayer::addAuxMessage(const QString &msg)
{
    ui->editLog->moveCursor(QTextCursor::End);
    ui->editLog->insertPlainText(QSL("%1 %2\n").arg(QTime::currentTime().toString(QSL("HH:mm:ss")),msg));
    ui->editLog->moveCursor(QTextCursor::End);
}

void ZSamplePlayer::updateSinkList()
{
    int savedIdx = -1;
    int defaultIdx = -1;
    QString savedPcm = ui->comboAlsaSink->currentText();

    ui->comboAlsaSink->clear();

    const auto pcms = m_renderArea->getAllPCMNames();
    const auto alsaPcms = gAlsa->pcmList();

    const auto allPcms = pcms + alsaPcms;
    if (!allPcms.contains(CPCMItem(QSL("default")))) {
        QString desc = tr("Automatically added default DSP");
        ui->comboAlsaSink->addItem(QSL("default"),desc);
        int lastIdx = ui->comboAlsaSink->count()-1;
        ui->comboAlsaSink->setItemData(lastIdx,desc,Qt::ToolTipRole);
        ui->comboAlsaSink->setItemData(lastIdx,1,Qt::UserRole+1);
        defaultIdx = lastIdx;
    }

    for (const auto& pcm : pcms) {
        QStringList desc = pcm.description;
        if (desc.isEmpty())
             desc.append(tr("DSP from editor"));
        ui->comboAlsaSink->addItem(pcm.name,desc);
        int lastIdx = ui->comboAlsaSink->count()-1;
        ui->comboAlsaSink->setItemData(lastIdx,0,Qt::UserRole+1);
        if (pcm.name == savedPcm)
            savedIdx = lastIdx;
    }
    for (const auto& pcm : alsaPcms) {
        ui->comboAlsaSink->addItem(pcm.name,pcm.description);
        int lastIdx = ui->comboAlsaSink->count()-1;
        ui->comboAlsaSink->setItemData(lastIdx,pcm.description.join(QSL("\n")),Qt::ToolTipRole);
        ui->comboAlsaSink->setItemData(lastIdx,1,Qt::UserRole+1);
        if (pcm.name == savedPcm && savedIdx<0)
            savedIdx = lastIdx;
        if (pcm.name.contains(QSL("default")) && defaultIdx<0)
            defaultIdx = lastIdx;
    }

    if (savedIdx>=0) {
        ui->comboAlsaSink->setCurrentIndex(savedIdx);
    } else if (defaultIdx>=0) {
        ui->comboAlsaSink->setCurrentIndex(defaultIdx);
    }
}

void ZSamplePlayer::updateLevelIndicator(double rmsL, double rmsR, double peakL, double peakR)
{
    ui->levelL->levelChanged(rmsL,peakL,8192);
    ui->levelR->levelChanged(rmsR,peakR,8192);
}

void ZSamplePlayer::updatePosition()
{
    ui->labelClock->clear();
    QString spos;
    if (m_data.pipeline) {
        if (gst_element_query(m_data.pipeline,m_data.positionQuery) == TRUE) {
            gint64 pos;
            gst_query_parse_position(m_data.positionQuery,nullptr,&pos);
            if (GST_CLOCK_TIME_IS_VALID(pos)) {
                gint64 s = (pos / GST_SECOND) % 60;
                gint64 m = (pos / (GST_SECOND * 60)) % 60;
                gint64 h = (pos / (GST_SECOND * 60 * 60));
                spos = QSL("%1:%2:%3").arg(h).arg(m,2,10,QChar('0')).arg(s,2,10,QChar('0'));
            }
        }
        if (!spos.isEmpty())
            ui->labelClock->setText(spos);
    }
}

void ZSamplePlayer::updateVolume(int value)
{
    if (m_data.volume) {
        gdouble vol = (2.0 - log10(100.0 - static_cast<double>(value))) / 2.0;
        g_object_set(m_data.volume, "volume", vol, nullptr);
    }
}

void ZSamplePlayer::getSinkInfo()
{
    if (m_data.alsasink) {
        gchar *card;
        gchar *deviceName;
        gchar *device;
        g_object_get(m_data.alsasink,
                     "card-name",&card,
                     "device-name",&deviceName,
                     "device",&device,nullptr);
        QString sdevice = QString::fromUtf8(device);
        addAuxMessage(tr(R"(APlugEdit: alsasink card: "%1", device name: "%2", ALSA output device: "%3".)").arg(
                          QString::fromUtf8(card),
                          QString::fromUtf8(deviceName),
                          sdevice));
        g_free(card);
        g_free(deviceName);
        g_free(device);

        // get hw_info
        QString cardId;
        unsigned int devNum;
        unsigned int subdevNum;
        if (gAlsa->getCardNumber(sdevice,cardId,&devNum,&subdevNum)) {
            QString procName = QSL("/proc/asound/%1/pcm%2p/sub%3/hw_params").arg(cardId).arg(devNum).arg(subdevNum);
            addAuxMessage(tr(R"(APlugEdit: parsed procfs path for device "%1" is "%2".)").arg(sdevice,procName));
            QFile f(procName);
            if (f.open(QIODevice::ReadOnly)) {
                QString info = tr("APlugEdit: card hardware info for selected device:\n");
                info.append(QString::fromUtf8(f.readAll()));
                addAuxMessage(info);
                f.close();
            } else {
                addAuxMessage(tr("APlugEdit: error: unable to open procfs file."));
            }
        } else {
            addAuxMessage(tr("APlugEdit: error: unable to get card info for selected device."));
        }
    }
}

bool ZSamplePlayer::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate) {
        updateSinkList();
    }
    return QDialog::event(event);
}

#endif // WITH_GST

