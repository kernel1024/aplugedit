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

#include "includes/generic.h"
#include "includes/sampleplayer_p.h"

void CStreamerData::clear()
{
    pipeline = nullptr;
    source = nullptr;
    audioconvert = nullptr;
    audioresample = nullptr;
    meter = nullptr;
    volume = nullptr;
    alsasink = nullptr;
    positionQuery = nullptr;
    busWatchID = 0;
}

ZSpecLogHighlighter::ZSpecLogHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{

}

void ZSpecLogHighlighter::highlightBlock(const QString &text)
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

void ZSpecLogHighlighter::formatBlock(const QString &text,
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

// Constants
const int RedrawInterval = 100; // ms
const double PeakDecayRate = 0.001;
const int PeakHoldLevelDuration = 2000; // ms

ZLevelMeter::ZLevelMeter(QWidget *parent)
    :   QWidget(parent)
    ,   m_peakDecayRate(PeakDecayRate)
    ,   m_redrawTimer(new QTimer(this))
    ,   m_rmsColor(0, 200, 0, 255)
    ,   m_peakColor(0, 255, 0, 255)
    ,   m_peakHoldColor(Qt::red)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setMinimumWidth(30);

    connect(m_redrawTimer, &QTimer::timeout, this, &ZLevelMeter::redrawTimerExpired);
    m_redrawTimer->start(RedrawInterval);

    m_peakLevelChanged.start();
    m_peakHoldLevelChanged.start();
}

ZLevelMeter::~ZLevelMeter() = default;

void ZLevelMeter::reset()
{
    m_rmsLevel = 0.0;
    m_peakLevel = 0.0;
    update();
}

void ZLevelMeter::levelChanged(double rmsLevel, double peakLevel, int numSamples)
{
    // Smooth the RMS signal
    const double smooth = pow(double(0.9), static_cast<double>(numSamples) / 256);
    m_rmsLevel = (m_rmsLevel * smooth) + (rmsLevel * (1.0 - smooth));

    if (peakLevel > m_decayedPeakLevel) {
        m_peakLevel = peakLevel;
        m_decayedPeakLevel = peakLevel;
        m_peakLevelChanged.start();
    }

    if (peakLevel > m_peakHoldLevel) {
        m_peakHoldLevel = peakLevel;
        m_peakHoldLevelChanged.start();
    }

    update();
}

void ZLevelMeter::redrawTimerExpired()
{
    // Decay the peak signal
    const auto elapsedMs = m_peakLevelChanged.elapsed();
    const double decayAmount = m_peakDecayRate * elapsedMs;
    if (decayAmount < m_peakLevel) {
        m_decayedPeakLevel = m_peakLevel - decayAmount;
    } else {
        m_decayedPeakLevel = 0.0;
    }

    // Check whether to clear the peak hold level
    if (m_peakHoldLevelChanged.elapsed() > PeakHoldLevelDuration)
        m_peakHoldLevel = 0.0;

    update();
}

void ZLevelMeter::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.save();

    painter.fillRect(rect(), Qt::black);

    QFont f = painter.font();
    f.setBold(true);
    painter.setFont(f);

    QRect bar = rect();

    bar.setTop(rect().top() + static_cast<int>((1.0 - m_peakHoldLevel) * rect().height()));
    bar.setBottom(bar.top() + 5);
    painter.fillRect(bar, m_peakHoldColor);
    bar.setBottom(rect().bottom());

    bar.setTop(rect().top() + static_cast<int>((1.0 - m_decayedPeakLevel) * rect().height()));
    painter.fillRect(bar, m_peakColor);

    bar.setTop(rect().top() + static_cast<int>((1.0 - m_rmsLevel) * rect().height()));
    painter.fillRect(bar, m_rmsColor);

    bar.setTop(rect().bottom() - 2 * painter.fontMetrics().height());
    painter.setPen(Qt::yellow);
    painter.drawText(bar,Qt::AlignCenter,objectName().right(1));

    painter.restore();
}

ZLogScale::ZLogScale(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setMinimumWidth(25);
}

ZLogScale::~ZLogScale() = default;

void ZLogScale::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const QList<int> labels({ 0, -5, -10, -20, -30, -40, -50, -60, -90 });

    QPainter p(this);
    p.save();

    QFont f = p.font();
    f.setPointSize(f.pointSize()-2);
    p.setFont(f);

    p.fillRect(rect(),palette().window());

    int vstep = height()/90;
    int idx = 0;
    while (idx*vstep<height()) {
        p.drawLine(width()/2 - 3, idx * vstep,
                   width()/2 + 3, idx * vstep);
        idx++;
    }

    vstep = (height() - p.fontMetrics().height()) / (labels.count() - 1);
    QRect tr(0,0,width(),vstep);
    for (int i=0;i<labels.count();i++) {
        QRect trb = tr;
        trb.setHeight(p.fontMetrics().height());
        p.fillRect(trb,palette().window());
        p.drawText(tr, Qt::AlignHCenter | Qt::AlignTop, QSL("%1").arg(labels.at(i)));
        tr.translate(0,vstep);
    }

    p.restore();
}

#endif // WITH_GST
