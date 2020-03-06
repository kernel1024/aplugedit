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

#ifndef SAMPLEPLAYER_H
#define SAMPLEPLAYER_H

#ifdef WITH_GST

#include <gst/gst.h>
#include <QDialog>
#include <QSyntaxHighlighter>

namespace Ui {
class ZSamplePlayer;
}

class CStreamerData
{
public:
    GstElement *pipeline { nullptr };
    GstElement *source { nullptr };
    GstElement *audioconvert { nullptr };
    GstElement *audioresample { nullptr };
    GstElement *alsasink { nullptr };
    guint busWatchID { 0 };
    CStreamerData() {}
    void clear();
};

class CSpecLogHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit CSpecLogHighlighter(QTextDocument* parent);
protected:
    void highlightBlock(const QString& text) override;
private:
    void formatBlock(const QString& text,
                     const QRegularExpression& exp,
                     const QColor& color = Qt::black,
                     bool weight = false,
                     bool italic = false,
                     bool underline = false,
                     bool strikeout = false);
};

class ZSamplePlayer : public QDialog
{
    Q_OBJECT

public:
    explicit ZSamplePlayer(QWidget *parent = nullptr);
    ~ZSamplePlayer() override;

public Q_SLOTS:
    void play();
    void stop();
    void browseFile();
    void addAuxMessage(const QString& msg);
    void updateSinkList();

private:
    Ui::ZSamplePlayer *ui;
    CSpecLogHighlighter *syntax;
    CStreamerData m_data;
    QStringList m_log;

    static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);
    static void pad_added_handler(GstElement *src, GstPad *new_pad, gpointer *data);
    static void addAuxMessageExt(ZSamplePlayer *instance, const QString& msg);
    static void debug_logger(GstDebugCategory *category, GstDebugLevel level, const gchar *file,
                             const gchar *function, gint line, GObject *object, GstDebugMessage *message,
                             gpointer user_data) G_GNUC_NO_INSTRUMENT;

    void initGstreamer();
    bool startGstreamer();


};

#endif // WITH_GST

#endif // SAMPLEPLAYER_H
