#include "includes/cpspeex.h"
#include "includes/generic.h"
#include "includes/speexdialog.h"

ZCPSpeex::ZCPSpeex(QWidget *parent, ZRenderArea *aOwner)
    : ZCPBase(parent,aOwner)
{
    fInp=new ZCPInput(this,this);
    fInp->pinName=QSL("in");
    registerInput(fInp);
    fOut=new ZCPOutput(this,this);
    fOut->pinName=QSL("out");
    registerOutput(fOut);
}

ZCPSpeex::~ZCPSpeex() = default;

void ZCPSpeex::readFromJson(const QJsonValue &json)
{
    ZCPBase::readFromJson(json.toObject().value(QSL("base")));

    m_denoise = ZGenericFuncs::readTristateFromJson(json.toObject().value(QSL("denoise")));
    m_agc = ZGenericFuncs::readTristateFromJson(json.toObject().value(QSL("agc")));
    m_dereverb = ZGenericFuncs::readTristateFromJson(json.toObject().value(QSL("dereverb")));
    m_echo = ZGenericFuncs::readTristateFromJson(json.toObject().value(QSL("echo")));

    m_frames = json.toObject().value(QSL("frames")).toInt(CDefaults::speexFrames);
    m_agcLevel = json.toObject().value(QSL("agcLevel")).toInt(CDefaults::speexAGCLevel);
    m_filterLength = json.toObject().value(QSL("filterLength")).toInt(CDefaults::speexFilterLength);
}

QJsonValue ZCPSpeex::storeToJson() const
{
    QJsonObject data;
    data.insert(QSL("base"), ZCPBase::storeToJson());

    data.insert(QSL("denoise"), ZGenericFuncs::writeTristateToJson(m_denoise));
    data.insert(QSL("agc"), ZGenericFuncs::writeTristateToJson(m_agc));
    data.insert(QSL("dereverb"), ZGenericFuncs::writeTristateToJson(m_dereverb));
    data.insert(QSL("echo"), ZGenericFuncs::writeTristateToJson(m_echo));

    data.insert(QSL("frames"), m_frames);
    data.insert(QSL("agcLevel"), m_agcLevel);
    data.insert(QSL("filterLength"), m_filterLength);

    return data;
}

QSize ZCPSpeex::minimumSizeHint() const
{
    return QSize(180,50);
}

void ZCPSpeex::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.save();

    paintBase(p);

    setBaseFont(p,ftTitle);
    p.drawText(rect(),Qt::AlignCenter,QSL("Speex"));

    QString s = QSL("Mono");
    if (m_denoise == Qt::CheckState::Checked)
        s.append(QSL(", noise"));
    if (m_agc == Qt::CheckState::Checked)
        s.append(QSL(", agc"));
    if (m_echo == Qt::CheckState::Checked)
        s.append(QSL(", echo"));
    setBaseFont(p,ftDesc);
    p.drawText(QRect(0,2*height()/3,width(),height()/3),Qt::AlignCenter,s);

    p.restore();
}

void ZCPSpeex::realignPins()
{
    fInp->relCoord=QPoint(zcpPinSize/2,height()/2);
    fOut->relCoord=QPoint(width()-zcpPinSize/2,height()/2);
}

void ZCPSpeex::doInfoGenerate(QTextStream &stream, QStringList &warnings) const
{
    stream << QSL("pcm.") << objectName() << QSL(" {") << endl;
    stream << QSL("  type speex") << endl;
    if (fOut->toFilter) {
        stream << QSL("  slave {") << endl;
        stream << QSL("    pcm \"") << fOut->toFilter->objectName() << QSL("\"") << endl;
        stream << QSL("  }") << endl;
    }
    if (m_frames>0)
        stream << QSL("  frames %1").arg(m_frames) << endl;
    if (m_agcLevel>0)
        stream << QSL("  agc_level %1").arg(m_agcLevel) << endl;
    if (m_filterLength>0)
        stream << QSL("  filter_length %1").arg(m_filterLength) << endl;

    if (m_denoise == Qt::CheckState::Checked) {
        stream << QSL("  denoise yes") << endl;
    } else if (m_denoise == Qt::CheckState::Unchecked) {
        stream << QSL("  denoise no") << endl;
    }
    if (m_agc == Qt::CheckState::Checked) {
        stream << QSL("  agc yes") << endl;
    } else if (m_agc == Qt::CheckState::Unchecked) {
        stream << QSL("  agc no") << endl;
    }
    if (m_dereverb == Qt::CheckState::Checked) {
        stream << QSL("  dereverb yes") << endl;
    } else if (m_dereverb == Qt::CheckState::Unchecked) {
        stream << QSL("  dereverb no") << endl;
    }
    if (m_echo == Qt::CheckState::Checked) {
        stream << QSL("  echo yes") << endl;
    } else if (m_echo == Qt::CheckState::Unchecked) {
        stream << QSL("  echo no") << endl;
    }

    ZCPBase::doInfoGenerate(stream,warnings);
    stream << QSL("}") << endl;
    stream << endl;
}

void ZCPSpeex::showSettingsDlg()
{
    ZSpeexDialog dlg(window());
    dlg.setParams(m_denoise,m_agc,m_dereverb,m_echo,m_frames,m_agcLevel,m_filterLength);

    if (dlg.exec() == QDialog::Rejected) return;

    dlg.getParams(m_denoise,m_agc,m_dereverb,m_echo,m_frames,m_agcLevel,m_filterLength);
}
