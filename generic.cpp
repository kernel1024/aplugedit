#include <QtWidgets>
#include <cmath>
#include <cfloat>
#include "includes/generic.h"
#include "ui_errorshowdlg.h"

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

void ZGenericFuncs::showWarningsDialog(QWidget *parent, const QString &title, const QString &text,
                                           const QStringList &warnings)
{
    QDialog dlg(parent);
    Ui::ZErrorShowDialog ui;
    ui.setupUi(&dlg);

    dlg.setWindowTitle(title);
    ui.label->setText(text);
    QIcon warnIcon = QIcon::fromTheme(QSL("dialog-warning"));
    ui.iconLabel->setPixmap(warnIcon.pixmap(64));
    for (const auto& s : warnings)
        ui.logViewer->append(s);
    dlg.setWindowModality(Qt::WindowModal);
    dlg.exec();
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

ZValidatedListEditDelegate::ZValidatedListEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ZValidatedListEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!(index.model()->data(index, Qt::UserRole).canConvert<QStringList>())) return nullptr;

    auto editor = new QComboBox(parent);
    editor->setFrame(false);

    return editor;
}

void ZValidatedListEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<QStringList>())) return;

    QString value = index.model()->data(index, Qt::EditRole).toString();
    QStringList items = index.model()->data(index, Qt::UserRole).toStringList();

    auto edit = qobject_cast<QComboBox*>(editor);
    edit->clear();
    edit->addItems(items);
    edit->setCurrentText(value);
}

void ZValidatedListEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<QStringList>())) return;

    auto edit = qobject_cast<QComboBox*>(editor);
    model->setData(index, edit->currentText(), Qt::EditRole);
}

void ZValidatedListEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                   const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

ZValidatedSpinBoxEditDelegate::ZValidatedSpinBoxEditDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ZValidatedSpinBoxEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                                     const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (!(index.model()->data(index, Qt::UserRole).canConvert<int>())) return nullptr;

    auto editor = new QSpinBox(parent);
    editor->setFrame(false);

    return editor;
}

void ZValidatedSpinBoxEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<int>())) return;

    bool ok1;
    bool ok2;
    int value = index.model()->data(index, Qt::EditRole).toInt(&ok1);
    int max = index.model()->data(index, Qt::UserRole).toInt(&ok2);

    auto edit = qobject_cast<QSpinBox*>(editor);
    edit->setMaximum(max);
    edit->setValue(value);
}

void ZValidatedSpinBoxEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                                 const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<int>())) return;

    auto edit = qobject_cast<QSpinBox*>(editor);
    model->setData(index, edit->value(), Qt::EditRole);
}

void ZValidatedSpinBoxEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
