#include <QtWidgets>
#include <cmath>
#include <cfloat>
#include "includes/generic.h"
#include "ui_errorshowdlg.h"

extern "C" {
#include <unistd.h>
}

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
            if (!(qApp->topLevelWidgets().isEmpty())) // NOLINT
                w = qApp->topLevelWidgets().first(); // NOLINT
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

Qt::CheckState ZGenericFuncs::readTristateFromJson(const QJsonValue &value)
{
    QString denoise = value.toString().toLower();
    if (denoise == QSL("yes") || denoise == QSL("on"))
        return Qt::CheckState::Checked;

    if (denoise == QSL("no") || denoise == QSL("off"))
        return Qt::CheckState::Unchecked;

    return Qt::CheckState::PartiallyChecked;
}

QJsonValue ZGenericFuncs::writeTristateToJson(Qt::CheckState state)
{
    switch (state) {
        case Qt::CheckState::Checked: return QJsonValue(QSL("yes"));
        case Qt::CheckState::Unchecked: return QJsonValue(QSL("no"));
        case Qt::CheckState::PartiallyChecked: return QJsonValue(QSL("default"));
    }
    return QJsonValue(QSL("undefined"));
}

bool ZGenericFuncs::runnedFromQtCreator()
{
    static int ppid = -1;
    static bool res = true;
    static QMutex mtx;
    QMutexLocker locker(&mtx);

    int tpid = getppid();
    if (tpid==ppid)
        return res;

    ppid = tpid;
    if (ppid>0) {
        QFileInfo fi(QFile::symLinkTarget(QSL("/proc/%1/exe").arg(ppid)));
        res = (fi.fileName().contains(QSL("creator"),Qt::CaseInsensitive) ||
               (fi.fileName().compare(QSL("gdb"))==0));
    }

    return res;
}

ZGenericFuncs::CommandLineParseResult ZGenericFuncs::parseCommandLine(QCommandLineParser &parser,
                                                                      QString* fileName,
                                                                      QString* errorMessage,
                                                                      bool* startMinimized)
{
    const QCommandLineOption minimizedOption({ QSL("m"), QSL("minimized") },
                                             QSL("Start application minimized to system tray."));
    parser.addOption(minimizedOption);
    parser.addPositionalArgument(QSL("filename"), QSL("The file to open in editor up."));
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (parser.isSet(helpOption))
        return CommandLineHelpRequested;

    *startMinimized = parser.isSet(minimizedOption);

    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() > 1) {
        *errorMessage = QSL("Several 'filename' arguments specified.");
        return CommandLineError;
    }
    if (!positionalArguments.isEmpty())
        *fileName = positionalArguments.first();

    return CommandLineOk;
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

    auto *editor = new QComboBox(parent);
    editor->setFrame(false);

    return editor;
}

void ZValidatedListEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<QStringList>())) return;

    QString value = index.model()->data(index, Qt::EditRole).toString();
    QStringList items = index.model()->data(index, Qt::UserRole).toStringList();

    auto *edit = qobject_cast<QComboBox*>(editor);
    edit->clear();
    edit->addItems(items);
    edit->setCurrentText(value);
}

void ZValidatedListEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                           const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<QStringList>())) return;

    auto *edit = qobject_cast<QComboBox*>(editor);
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

    auto *editor = new QSpinBox(parent);
    editor->setFrame(false);

    return editor;
}

void ZValidatedSpinBoxEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<int>())) return;

    bool ok1 = 0;
    bool ok2 = 0;
    int value = index.model()->data(index, Qt::EditRole).toInt(&ok1);
    int max = index.model()->data(index, Qt::UserRole).toInt(&ok2);

    auto *edit = qobject_cast<QSpinBox*>(editor);
    edit->setMaximum(max);
    edit->setValue(value);
}

void ZValidatedSpinBoxEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                                 const QModelIndex &index) const
{
    if (!(index.model()->data(index, Qt::UserRole).canConvert<int>())) return;

    auto *edit = qobject_cast<QSpinBox*>(editor);
    model->setData(index, edit->value(), Qt::EditRole);
}

void ZValidatedSpinBoxEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
