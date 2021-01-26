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

#include <QMetaEnum>
#include <QMimeData>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QDoubleSpinBox>
#include "includes/ladspa_p.h"
#include "includes/generic.h"
#include "includes/ladspadialog.h"

ZResizableFrame::ZResizableFrame(QWidget *parent)
    : QFrame(parent)
{
    m_scroller=qobject_cast<QScrollArea*>(parent);
}

QSize ZResizableFrame::minimumSizeHint() const
{
    QRect r(0,0,width(),0);
    for (int i=0;i<children().count();i++)
    {
        QWidget* w=qobject_cast<QWidget*>(children().at(i));
        if (!w) continue;
        if ((qobject_cast<QCheckBox*>(w)) || (qobject_cast<QDoubleSpinBox*>(w)))
        {
            r.adjust(0,0,0,7*w->sizeHint().height()/5);
        }
    }
    if (r.height()==0)
        return QSize(r.width(),r.height());

    return size().expandedTo(QSize(r.width(),r.height()));
}

QSize ZResizableFrame::sizeHint() const
{
    return minimumSizeHint();
}

CLADSPAControlItem::CLADSPAControlItem(const QString &AportName, ZLADSPA::Control AaatType,
                                       bool AaasToggle, double AaasValue, QWidget* AaawControl,
                                       QLayout* AaawLayout, QLabel* AaawLabel)
{
    aatType=AaatType;
    aasToggle=AaasToggle;
    aasValue=AaasValue;
    aasFreq=ZGenericFuncs::truncDouble(aasValue);
    aasInt=ZGenericFuncs::truncDouble(aasValue);
    portName=AportName;
    aawControl=AaawControl;
    aawLayout=AaawLayout;
    aawLabel=AaawLabel;
}

QJsonValue CLADSPAControlItem::storeToJson() const
{
    QJsonObject data;

    auto typeEnum = QMetaEnum::fromType<ZLADSPA::Control>();
    data.insert(QSL("type"),typeEnum.valueToKey(aatType));

    data.insert(QSL("toggle"),static_cast<int>(aasToggle));
    data.insert(QSL("value"),aasValue);
    data.insert(QSL("freq"),aasFreq);
    data.insert(QSL("int"),aasInt);
    data.insert(QSL("portName"),portName);

    return data;
}

void CLADSPAControlItem::destroyControls()
{
    if (aawControl)
        aawControl->deleteLater();
    if (aawLayout)
        aawLayout->deleteLater();
    if (aawLabel)
        aawLabel->deleteLater();
    disconnectFromControls();
}

void CLADSPAControlItem::disconnectFromControls()
{
    aawControl=nullptr;
    aawLayout=nullptr;
    aawLabel=nullptr;
}

CLADSPAControlItem::CLADSPAControlItem(QDataStream &s)
{
    s.readRawData(reinterpret_cast<char*>(&(aatType)),sizeof(aatType));
    s >> aasToggle >> aasValue >> aasFreq >> aasInt >> portName;
}

CLADSPAControlItem::CLADSPAControlItem(const QJsonValue &json)
{
    auto typeEnum = QMetaEnum::fromType<ZLADSPA::Control>();
    aatType = static_cast<ZLADSPA::Control>(typeEnum.keyToValue(json.toObject().value(QSL("type"))
                                                                .toString().toLatin1().constData()));

    aasToggle = static_cast<bool>(json.toObject().value(QSL("toggle")).toInt(0));
    aasValue = json.toObject().value(QSL("value")).toDouble(0.0);
    aasFreq = json.toObject().value(QSL("freq")).toInt(0);
    aasInt = json.toObject().value(QSL("int")).toInt(0);
    portName = json.toObject().value(QSL("portName")).toString();
}

CLADSPAPlugItem::CLADSPAPlugItem(const QJsonValue &json)
{
    plugName = json.toObject().value(QSL("name")).toString();
    plugID = static_cast<qint64>(json.toObject().value(QSL("ID")).toInt(0));
    plugLabel = json.toObject().value(QSL("label")).toString();
    plugLibrary = json.toObject().value(QSL("library")).toString();

    usePolicy = json.toObject().value(QSL("usePolicy")).toBool(false);
    policy = ZLADSPA::Policy::plDuplicate;
    if (json.toObject().value(QSL("policy")).toString() == QSL("none"))
        policy = ZLADSPA::Policy::plNone;

    const QJsonArray jinputs = json.toObject().value(QSL("inputBindings")).toArray();
    for (const auto& inp : jinputs) {
        int ch = inp.toObject().value(QSL("channel")).toInt(-1);
        QString pin = inp.toObject().value(QSL("pin")).toString();
        if (ch>=0 && !pin.isEmpty())
            inputBindings.append(qMakePair(ch, pin));
    }

    const QJsonArray joutputs = json.toObject().value(QSL("outputBindings")).toArray();
    for (const auto& outp : joutputs) {
        int ch = outp.toObject().value(QSL("channel")).toInt(-1);
        QString pin = outp.toObject().value(QSL("pin")).toString();
        if (ch>=0 && !pin.isEmpty())
            outputBindings.append(qMakePair(ch, pin));
    }

    const QJsonArray jcontrols = json.toObject().value(QSL("controls")).toArray();
    plugControls.reserve(jcontrols.count());
    for (const auto& item : jcontrols)
        plugControls.append(CLADSPAControlItem(item));
}

CLADSPAPlugItem::CLADSPAPlugItem(const QString &AplugLabel, qint64 AplugID, const QString &AplugName,
                                 const QString &AplugLibrary, const QVector<CLADSPAControlItem> &aPlugControls, bool aUsePolicy,
                                 ZLADSPA::Policy aPolicy, const CInOutBindings &aInputBindings,
                                 const CInOutBindings &aOutputBindings)
{
    plugLabel = AplugLabel;
    plugID = AplugID;
    plugName = AplugName;
    plugLibrary = AplugLibrary;
    plugControls = aPlugControls;
    usePolicy = aUsePolicy;
    policy = aPolicy;
    inputBindings = aInputBindings;
    outputBindings = aOutputBindings;
}

QJsonValue CLADSPAPlugItem::storeToJson() const
{
    QJsonObject jplug;
    jplug.insert(QSL("name"),plugName);
    jplug.insert(QSL("ID"),static_cast<int>(plugID));
    jplug.insert(QSL("label"),plugLabel);
    jplug.insert(QSL("library"),plugLibrary);
    jplug.insert(QSL("usePolicy"),usePolicy);
    if (policy==ZLADSPA::Policy::plNone) {
        jplug.insert(QSL("policy"),QSL("none"));
    } else {
        jplug.insert(QSL("policy"),QSL("duplicate"));
    }

    QJsonArray jinputs;
    for (const auto &inp : qAsConst(inputBindings)) {
        QJsonObject jinp;
        jinp.insert(QSL("channel"),inp.first);
        jinp.insert(QSL("pin"),inp.second);
        jinputs.append(jinp);
    }
    jplug.insert(QSL("inputBindings"),jinputs);

    QJsonArray joutputs;
    for (const auto &outp : qAsConst(outputBindings)) {
        QJsonObject jout;
        jout.insert(QSL("channel"),outp.first);
        jout.insert(QSL("pin"),outp.second);
        joutputs.append(jout);
    }
    jplug.insert(QSL("outputBindings"),joutputs);

    QJsonArray controls;
    for (int i=0;i<plugControls.count();i++)
        controls.append(plugControls[i].storeToJson());
    jplug.insert(QSL("controls"),controls);

    return jplug;
}

QStringList CLADSPAPlugItem::getBindingsDesc() const
{
    const QChar arrow(0x2192);

    QStringList res;
    QString s = QSL("Inputs: ");
    bool isFirst = true;
    for (const auto& inp : qAsConst(inputBindings)) {
        if (!isFirst)
            s.append(QSL(", "));
        s.append(QSL("%1%2%3").arg(inp.first).arg(arrow).arg(inp.second));
        isFirst = false;
    }
    res << s;
    s = QSL("Outputs: ");
    isFirst = true;
    for (const auto& outp : qAsConst(outputBindings)) {
        if (!isFirst)
            s.append(QSL(", "));
        s.append(QSL("%1%2%3").arg(outp.first).arg(arrow).arg(outp.second));
        isFirst = false;
    }
    res << s;
    return res;
}

bool CLADSPAPlugItem::isEmpty() const
{
    return (plugID == 0);
}

ZLADSPABindingsModel::ZLADSPABindingsModel(ZLADSPADialog *parent)
    : QAbstractTableModel(parent)
    , m_mainDialog(parent)
{
}

ZLADSPABindingsModel::~ZLADSPABindingsModel() = default;

CInOutBindings ZLADSPABindingsModel::getBindings() const
{
    return m_bindings;
}

void ZLADSPABindingsModel::setBindings(const CInOutBindings &bindings)
{
    removeRows(0,rowCount());

    if (bindings.isEmpty()) return;

    beginInsertRows(QModelIndex(), 0, bindings.count()-1);
    m_bindings = bindings;
    endInsertRows();
}

void ZLADSPABindingsModel::setValidPorts(const QStringList &validPorts)
{
    m_validPorts = validPorts;

    if (m_validPorts.isEmpty()) {
        removeRows(0,rowCount());
        return;
    }

    for (int i=0;i<rowCount();i++) {
        if (!m_validPorts.contains(m_bindings.at(i).second)) {
            m_bindings[i].second = m_validPorts.first();
            Q_EMIT dataChanged(index(i,1),index(i,1));
        }
    }
}

Qt::ItemFlags ZLADSPABindingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;

    return flags;
}

QVariant ZLADSPABindingsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
                case 0: return QSL("%1").arg(m_bindings.at(index.row()).first);
                case 1: return m_bindings.at(index.row()).second;
                default: return QVariant();
            }
        case Qt::UserRole: // valid values
            if (index.column()==1)
                return m_validPorts;
            break;
        default:
            break;
    }

    return QVariant();
}

int ZLADSPABindingsModel::rowCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return m_bindings.count();
}

int ZLADSPABindingsModel::columnCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return 2;
}

QVariant ZLADSPABindingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QSL("ALSA channel");
            case 1: return QSL("Plugin port");
            default: return QVariant();
        }
    }
    return QVariant();
}

bool ZLADSPABindingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return false;

    int row = index.row();
    int col = index.column();

    if (role == Qt::EditRole) {
        QString s = value.toString();
        switch (col) {
            case 0: {
                bool ok = 0;
                int idx = s.toInt(&ok);
                if (ok)
                    m_bindings[row].first = idx;
                Q_EMIT dataChanged(index,index);
                return true;
            }
            case 1:
                m_bindings[row].second = s;
                Q_EMIT dataChanged(index,index);
                return true;
            default: break;
        }
    }
    return false;
}

void ZLADSPABindingsModel::appendRow()
{
    insertRows(rowCount(),1);
}

void ZLADSPABindingsModel::deleteRow(const QModelIndex &index)
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return;

    removeRows(index.row(),1);
}

bool ZLADSPABindingsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    if (m_validPorts.isEmpty()) return false;

    // unmapped channel autoincrement
    int maxc = 0;
    for (const auto& c : qAsConst(m_bindings))
        maxc = qMax(maxc,c.first+1);
    maxc = qMin(maxc,m_mainDialog->getChannels());

    beginInsertRows(QModelIndex(), row, row+count-1);
    for (int i=0;i<count;i++)
        m_bindings.insert(row,qMakePair(maxc,qAsConst(m_validPorts).first()));
    endInsertRows();
    return true;
}

bool ZLADSPABindingsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    beginRemoveRows(parent, row, row+count-1);
    m_bindings.remove(row,count);
    endRemoveRows();
    return true;
}

bool ZLADSPABindingsModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

bool ZLADSPABindingsModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(column)
    Q_UNUSED(count)
    Q_UNUSED(parent)
    return false;
}

ZLADSPAListModel::ZLADSPAListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_mimeType(QSL("application/ladspa-plugin-list"))
{
}

ZLADSPAListModel::~ZLADSPAListModel() = default;

void ZLADSPAListModel::setItems(const QVector<CLADSPAPlugItem> &items)
{
    deleteAllItems();
    addItems(items);
}

void ZLADSPAListModel::addItems(const QVector<CLADSPAPlugItem> &items)
{
    int posidx = m_items.count();
    insertItems(posidx,items);
}

void ZLADSPAListModel::insertItems(int row, const QVector<CLADSPAPlugItem> &items)
{
    beginInsertRows(QModelIndex(), row, row+m_items.count()-1);
    m_items.append(items);
    endInsertRows();
}

void ZLADSPAListModel::deleteAllItems()
{
    beginRemoveRows(QModelIndex(),0,m_items.count()-1);
    m_items.clear();
    endRemoveRows();
}

void ZLADSPAListModel::addItem(const CLADSPAPlugItem &item)
{
    addItems({ item });
}

void ZLADSPAListModel::insertItem(int row, const CLADSPAPlugItem &item)
{
    insertItems(row,{ item });
}

void ZLADSPAListModel::setItem(int row, const CLADSPAPlugItem &item)
{
    m_items[row] = item;
    Q_EMIT dataChanged(index(row),index(row));
}

Qt::ItemFlags ZLADSPAListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;

    if (!index.isValid())
        return flags;

    flags = flags | Qt::ItemIsDragEnabled;

    return flags;
}

QVariant ZLADSPAListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return QVariant();

    int idx = index.row();
    switch (role) {
        case Qt::DisplayRole:
            return QSL("%1 (%2/%3)")
                    .arg(m_items.at(idx).plugName)
                    .arg(m_items.at(idx).plugID)
                    .arg(m_items.at(idx).plugLabel);
        case Qt::UserRole:
            return m_items.at(idx).getBindingsDesc();
        default:
            break;
    }

    return QVariant();
}

int ZLADSPAListModel::rowCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return m_items.count();
}

bool ZLADSPAListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return insertRowsPriv(row, count, parent);
}

bool ZLADSPAListModel::insertRowsPriv(int row, int count, const QModelIndex &parent, const QVector<CLADSPAPlugItem> &items)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    int cnt = count;
    if (!items.isEmpty())
        cnt = items.count();

    beginInsertRows(QModelIndex(), row, row+cnt-1);
    for (int i=(count-1);i>=0;i--) {
        if (!items.isEmpty()) {
            m_items.insert(row,items.at(i));
        } else {
            m_items.insert(row,CLADSPAPlugItem());
        }
    }
    endInsertRows();
    return true;
}

bool ZLADSPAListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    beginRemoveRows(parent, row, row+count-1);
    m_items.remove(row,count);
    endRemoveRows();
    return true;
}

QMimeData *ZLADSPAListModel::mimeData(const QModelIndexList &indexes) const
{
    auto *mimeData = new QMimeData();
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    for (const QModelIndex& index : indexes) {
        if (index.isValid())
            stream << item(index);
    }
    mimeData->setData(m_mimeType, data);
    return mimeData;
}

QStringList ZLADSPAListModel::mimeTypes() const
{
    return QStringList({ m_mimeType });
}

bool ZLADSPAListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                    const QModelIndex &parent)
{
    Q_UNUSED(column)

    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(m_mimeType))
        return false;

    QByteArray ba = data->data(m_mimeType);
    QDataStream stream(&ba, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    QVector<CLADSPAPlugItem> items;
    while (!stream.atEnd()) {
        CLADSPAPlugItem item;
        stream >> item;
        items.append(item);
    }

    int beginRow = 0;
    if (row != -1) {
        beginRow = row;
    } else {
        beginRow = rowCount();
    }

    insertRowsPriv(beginRow, items.count(), QModelIndex(), items);

    return true;
}

bool ZLADSPAListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                                       const QModelIndex &parent) const
{
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)

    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    if (!data->hasFormat(m_mimeType))
        return false;

    return true;
}

Qt::DropActions ZLADSPAListModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

int ZLADSPAListModel::getRowIndex(const QModelIndex &index) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return -1;

    return index.row();
}

QVector<CLADSPAPlugItem> ZLADSPAListModel::items() const
{
    return m_items;
}

CLADSPAPlugItem ZLADSPAListModel::item(const QModelIndex &index) const
{
    int row = getRowIndex(index);
    if (row>=0)
        return m_items.at(row);

    return CLADSPAPlugItem();
}

QDataStream &operator<<(QDataStream &out, const CLADSPAPlugItem &item)
{
    out << item.usePolicy << item.policy << item.plugID << item.plugLabel << item.plugName
        << item.plugLibrary << item.inputBindings << item.outputBindings << item.plugControls;
    return out;
}

QDataStream &operator>>(QDataStream &in, CLADSPAPlugItem &item)
{
    in >> item.usePolicy >> item.policy >> item.plugID >> item.plugLabel >> item.plugName
       >> item.plugLibrary >> item.inputBindings >> item.outputBindings >> item.plugControls;
    return in;
}

QDataStream &operator<<(QDataStream &out, const CLADSPAControlItem &item)
{
    out << item.aatType << item.aasToggle << item.aasValue << item.aasFreq << item.aasInt << item.portName;
    return out;
}

QDataStream &operator>>(QDataStream &in, CLADSPAControlItem &item)
{
    in >> item.aatType >> item.aasToggle >> item.aasValue >> item.aasFreq >> item.aasInt >> item.portName;
    item.aawControl = nullptr;
    item.aawLayout = nullptr;
    item.aawLabel = nullptr;
    return in;
}
