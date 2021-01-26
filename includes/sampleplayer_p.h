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

#ifndef SAMPLEPLAYER_P_H
#define SAMPLEPLAYER_P_H

#ifdef WITH_GST

#include <gst/gst.h>
#include <QTimer>
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QElapsedTimer>

class CStreamerData
{
public:
    GstElement *pipeline { nullptr };
    GstElement *source { nullptr };
    GstElement *audioconvert { nullptr };
    GstElement *audioresample { nullptr };
    GstElement *meter { nullptr };
    GstElement *volume { nullptr };
    GstElement *alsasink { nullptr };
    GstQuery *positionQuery { nullptr };
    guint busWatchID { 0 };
    CStreamerData() {}
    void clear();
};

class ZSpecLogHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit ZSpecLogHighlighter(QTextDocument* parent);

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

class ZLogScale : public QWidget
{
    Q_OBJECT
public:
    ZLogScale(QWidget *parent = nullptr);
    ~ZLogScale() override;
protected:
    void paintEvent(QPaintEvent *event) override;
};

class ZLevelMeter : public QWidget
{
    Q_OBJECT

public:
    explicit ZLevelMeter(QWidget *parent = nullptr);
    ~ZLevelMeter() override;

    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void reset();
    void levelChanged(double rmsLevel, double peakLevel, int numSamples);

private Q_SLOTS:
    void redrawTimerExpired();

private:
    double m_rmsLevel { 0.0 };
    double m_peakLevel { 0.0 };
    double m_decayedPeakLevel { 0.0 };
    double m_peakDecayRate { 0.0 };
    double m_peakHoldLevel { 0.0 };
    QElapsedTimer m_peakLevelChanged;
    QElapsedTimer m_peakHoldLevelChanged;

    QTimer *m_redrawTimer;

    QColor m_rmsColor;
    QColor m_peakColor;
    QColor m_peakHoldColor;

};

#endif // WITH_GST

#endif // SAMPLEPLAYER_P_H
