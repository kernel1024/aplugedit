#include <QtWidgets>
#include <cmath>
#include <cfloat>
#include "includes/generic.h"

ZGenericFuncs::ZGenericFuncs(QObject *parent)
    : QObject(parent)
{

}

ZGenericFuncs::~ZGenericFuncs() = default;

int ZGenericFuncs::numDigits(int n) {
    const int base = 10;
    const int minusBase = ((-1)*base)+1;

    if ((n >= 0) && (n < base))
        return 1;

    if ((n >= minusBase) && (n < 0))
        return 2;

    if (n<0)
        return 2 + numDigits(std::abs(n) / base);

    return 1 + numDigits(n / base);
}

int ZGenericFuncs::truncDouble(double num)
{
    return static_cast<int>(std::trunc(num));
}

QString ZGenericFuncs::getLADSPAPath()
{
    static QString ladspa_path;
    if (ladspa_path.isEmpty()) {
        ladspa_path=qEnvironmentVariable("LADSPA_PATH");
        if (ladspa_path.isEmpty()) {
            ladspa_path = QSL("/usr/lib/ladspa:/usr/local/lib/ladspa");
            QWidget* w = nullptr;
            if (!(qApp->topLevelWidgets().isEmpty()))
                w = qApp->topLevelWidgets().first();
            QMessageBox::warning(w,tr("LADSPA warning"),
                                 tr("Warning: You do not have a LADSPA_PATH environment variable set.\n"
                                    "Defaulting to /usr/lib/ladspa, /usr/local/lib/ladspa."));
        }
    }
    return ladspa_path;
}

ZDescListItemDelegate::ZDescListItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

ZDescListItemDelegate::~ZDescListItemDelegate() = default;

void ZDescListItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    const bool selected = ((option.state & QStyle::State_Selected) > 0);
    if (selected) {
        painter->fillRect(option.rect,option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    } else {
        painter->fillRect(option.rect,option.palette.base());
        painter->setPen(option.palette.text().color());
    }

    if (index.isValid()) {
        QString name = index.model()->data(index,Qt::DisplayRole).toString();
        QStringList desc = index.model()->data(index,Qt::UserRole).toStringList();
        QVariant vinternal = index.model()->data(index,Qt::UserRole+1);
        if (!name.isEmpty()) {
            QFont f = option.font;
            f.setBold(true);
            if (vinternal.isValid() && (vinternal.toInt() == 0)) {
                f.setItalic(true);
                name.append(tr(" (editor)"));
            }
            painter->setFont(f);

            QRect r = option.rect;
            r.adjust(5,0,-5,0);
            r.setHeight(r.height()/3);
            painter->drawText(r,Qt::AlignLeft | Qt::AlignVCenter,name);

            if (!selected)
                painter->setPen(Qt::darkGray);

            f = option.font;
            f.setPointSize(f.pointSize()-3);
            painter->setFont(f);
            r.translate(0,r.height());
            r.setHeight(2*option.rect.height()/3);
            painter->drawText(r,Qt::AlignLeft | Qt::AlignVCenter,desc.join(QSL("\n")));
        }
    }

    painter->restore();
}

QSize ZDescListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(100,3*option.fontMetrics.height());
}
