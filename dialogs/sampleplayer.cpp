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

ZSamplePlayer::ZSamplePlayer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZSamplePlayer)
{
    ui->setupUi(this);

    syntax = new CSpecLogHighlighter(ui->editLog->document());

    connect(ui->buttonPlay,&QPushButton::clicked,this,&ZSamplePlayer::play);
    connect(ui->buttonStop,&QPushButton::clicked,this,&ZSamplePlayer::stop);
    connect(ui->buttonBrowse,&QPushButton::clicked,this,&ZSamplePlayer::browseFile);
    connect(ui->editLog, &QTextEdit::textChanged,this,[this](){
        ui->linesCount->setText(tr("%1 messages").arg(ui->editLog->document()->lineCount() - 1));
    });

    QSettings stg;
    stg.beginGroup(QSL("SamplePlayer"));
    ui->editFilename->setText(stg.value(QSL("sampleFile"),QString()).toString());
    ui->comboWaveform->setCurrentText(stg.value(QSL("generatorWaveform"),QSL("sine")).toString());
    ui->spinFrequency->setValue(stg.value(QSL("generatorFreq"),440.0).toDouble());

    ui->radioFuncGenerator->setChecked(true); // force widged disabling logic
    ui->radioMediaFile->setChecked(!(stg.value(QSL("funcGenerator"),false).toBool()));
    stg.endGroup();

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
    m_data.alsasink = gst_element_factory_make("alsasink", "sink");

    if (!m_data.pipeline || !m_data.source || !m_data.audioconvert
            || !m_data.audioresample || !m_data.alsasink) {
        addAuxMessage(tr("APlugEdit: one of pipeline element could not be created."));
        return false;
    }

    /* Set up the pipeline */

    /* we add all elements into the pipeline */
    gst_bin_add_many(GST_BIN(m_data.pipeline),m_data.source,m_data.audioconvert,
                     m_data.audioresample,m_data.alsasink,nullptr);
    if (gst_element_link_many(m_data.audioconvert, m_data.audioresample, m_data.alsasink, nullptr) == FALSE) {
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

    QByteArray bsink = ui->comboAlsaSink->currentText().toLatin1();
    if (!bsink.isEmpty())
        g_object_set(m_data.alsasink,"device", bsink.constData(), nullptr);

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_data.pipeline));
    m_data.busWatchID = gst_bus_add_watch(bus, bus_call, this);
    gst_object_unref(bus);

    /* Start playing */
    GstStateChangeReturn ret = gst_element_set_state(m_data.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        addAuxMessage(tr("APlugEdit: Unable to set the pipeline to the playing state."));
        stop();
        return false;
    }

    // TODO: catch play progress events, display alsasink.device-name and .card-name

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

        gst_object_unref(GST_OBJECT(m_data.pipeline));

        if (m_data.busWatchID>0)
            g_source_remove(m_data.busWatchID);

        m_data.clear();

        gst_debug_add_log_function(gst_debug_log_default,stderr,nullptr);
        gst_debug_remove_log_function(debug_logger);
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
    ui->comboAlsaSink->clear();

    // TODO: show description with delegate
    // TODO: add all DSP's from editor (even without hint) - aquire list from MainWindow via config write signal?
    const auto pcms = gAlsa->pcmList();
    for (const auto& pcm : pcms) {
        ui->comboAlsaSink->addItem(pcm.name);
    }
}

void CStreamerData::clear()
{
    pipeline = nullptr;
    source = nullptr;
    audioconvert = nullptr;
    audioresample = nullptr;
    alsasink = nullptr;
    busWatchID = 0;
}

CSpecLogHighlighter::CSpecLogHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{

}

void CSpecLogHighlighter::highlightBlock(const QString &text)
{
    formatBlock(text,QRegularExpression(QSL("^\\S{,8}"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::black,true);
    formatBlock(text,QRegularExpression(QSL("\\s(DEBUG|MSG):\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::black,true);
    formatBlock(text,QRegularExpression(QSL("\\sWARN:\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::darkRed,true);
    formatBlock(text,QRegularExpression(QSL("\\sERROR:\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::red,true);
    formatBlock(text,QRegularExpression(QSL("\\sINFO:\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::darkBlue,true);
    formatBlock(text,QRegularExpression(QSL("\\s(FIXME|LOG|TRACE|MEMDUMP):\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::darkCyan,true);
    formatBlock(text,QRegularExpression(QSL("\\sSTATE:\\s"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::blue,true);
    formatBlock(text,QRegularExpression(QSL("\\sAPlugEdit:\\s"),
                                        QRegularExpression::NoPatternOption),Qt::black,true);
    formatBlock(text,QRegularExpression(QSL("\\(\\S+\\)$"),
                                        QRegularExpression::CaseInsensitiveOption),Qt::gray,false,true);
}

void CSpecLogHighlighter::formatBlock(const QString &text,
                                      const QRegularExpression &exp,
                                      const QColor &color,
                                      bool weight,
                                      bool italic,
                                      bool underline,
                                      bool strikeout)
{
    if (text.isEmpty()) return;

    QTextCharFormat fmt;
    fmt.setForeground(color);
    if (weight) {
        fmt.setFontWeight(QFont::Bold);
    } else {
        fmt.setFontWeight(QFont::Normal);
    }
    fmt.setFontItalic(italic);
    fmt.setFontUnderline(underline);
    fmt.setFontStrikeOut(strikeout);

    auto it = exp.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), fmt);
    }
}

#endif // WITH_GST

