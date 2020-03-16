#include "includes/multidlg.h"
#include "includes/generic.h"
#include "ui_multidlg.h"

ZMultiDialog::ZMultiDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZMultiDialog)
{
    ui->setupUi(this);

    m_outputsModel = new ZMultiOutputsModel(this);
    ui->tableOutputChannels->setModel(m_outputsModel);
    m_bindingsModel = new ZMultiBindingsModel(this,m_outputsModel);
    ui->tableBindings->setModel(m_bindingsModel);
    ui->tableBindings->setItemDelegate(new ZValidatedListEditDelegate(this));
    ui->tableOutputChannels->setItemDelegate(new ZValidatedSpinBoxEditDelegate(this));

    connect(ui->spinCapacity,qOverload<int>(&QSpinBox::valueChanged),
            m_bindingsModel,&ZMultiBindingsModel::capacityChanged);
    connect(ui->spinOutputsCount,qOverload<int>(&QSpinBox::valueChanged),
            m_outputsModel,&ZMultiOutputsModel::outputsCountChanged);
    connect(m_outputsModel,&ZMultiOutputsModel::outputChannelsChanged,
            m_bindingsModel,&ZMultiBindingsModel::outputsChanged);
}

ZMultiDialog::~ZMultiDialog()
{
    delete ui;
}

void ZMultiDialog::setParams(const QVector<CMultiBinding> &bindings, const QVector<int> &slaveChannels)
{
    ui->spinCapacity->blockSignals(true);
    ui->spinOutputsCount->blockSignals(true);
    ui->spinCapacity->setValue(bindings.count());
    ui->spinOutputsCount->setValue(slaveChannels.count());
    ui->spinCapacity->blockSignals(false);
    ui->spinOutputsCount->blockSignals(false);
    m_outputsModel->setSlaveChannels(slaveChannels);
    m_bindingsModel->setBindings(bindings);
}

void ZMultiDialog::getParams(QVector<CMultiBinding> &bindings, QVector<int> &slaveChannels)
{
    bindings = m_bindingsModel->getBindings();
    slaveChannels = m_outputsModel->getSlaveChannels();
}

QString ZMultiDialog::formatOutputName(int idx)
{
    return QSL("out#%1").arg(idx);
}

ZMultiBindingsModel::ZMultiBindingsModel(QObject *parent, ZMultiOutputsModel *outputsModel)
    : QAbstractTableModel(parent)
    , m_outputsModel(outputsModel)
{
}

ZMultiBindingsModel::~ZMultiBindingsModel() = default;

Qt::ItemFlags ZMultiBindingsModel::flags(const QModelIndex &index) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column()>0)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant ZMultiBindingsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
                case 0: return QSL("%1").arg(index.row());
                case 1: return ZMultiDialog::formatOutputName(m_bindings.at(index.row()).first);
                case 2: return QSL("%1").arg(m_bindings.at(index.row()).second);
                default: return QVariant();
            }
        case Qt::UserRole: // valid values
            if (index.column()==1) {
                QStringList sl;
                sl.reserve(m_outputsModel->rowCount());
                for (int i=0;i<m_outputsModel->rowCount();i++)
                    sl.append(ZMultiDialog::formatOutputName(i));
                return sl;
            } else if (index.column()==2) {
                QStringList sl;
                for (int i=0;i<m_outputsModel->getSlaveChannelsCount(
                         m_bindings.at(index.row()).first);i++)
                    sl.append(QSL("%1").arg(i));
                return sl;
            }
            break;
        default:
            break;
    }

    return QVariant();
}

int ZMultiBindingsModel::rowCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return m_bindings.count();
}

int ZMultiBindingsModel::columnCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return 3;
}

QVariant ZMultiBindingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QSL("Input channel");
            case 1: return QSL("Slave PCM");
            case 2: return QSL("Slave channel");
            default: return QVariant();
        }
    }
    return QVariant();
}

bool ZMultiBindingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return false;

    int row = index.row();
    int col = index.column();

    if (role == Qt::EditRole) {
        QString s = value.toString();
        switch (col) {
            case 1: {
                s.remove(0,s.indexOf('#')+1);
                bool ok;
                int outIdx = s.toInt(&ok);
                if (ok) {
                    m_bindings[row].first = outIdx;
                    Q_EMIT dataChanged(index,index);
                }
                return true;
            }
            case 2: {
                bool ok;
                int outChannel = s.toInt(&ok);
                if (ok) {
                    m_bindings[row].second = outChannel;
                    Q_EMIT dataChanged(index,index);
                }
                return true;
            }
            default: break;
        }
    }
    return false;
}

bool ZMultiBindingsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    beginInsertRows(QModelIndex(), row, row+count-1);
    for (int i=0;i<count;i++) {
        int outIdx = 0;
        int outChannel = 0;
        m_bindings.insert(row,qMakePair(outIdx,outChannel));
    }
    endInsertRows();
    return true;
}

bool ZMultiBindingsModel::removeRows(int row, int count, const QModelIndex &parent)
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

void ZMultiBindingsModel::setBindings(const QVector<CMultiBinding> &bindings)
{
    removeRows(0,rowCount());
    beginInsertRows(QModelIndex(),0,bindings.count()-1);
    m_bindings = bindings;
    endInsertRows();
}

QVector<CMultiBinding> ZMultiBindingsModel::getBindings() const
{
    return m_bindings;
}

void ZMultiBindingsModel::capacityChanged(int value)
{
    if (value<rowCount())
        removeRows(value,rowCount()-value);

    if (value>rowCount())
        insertRows(rowCount(),value-rowCount());
}

void ZMultiBindingsModel::outputsChanged()
{
    for (int row=0;row<rowCount();row++) {
        if (m_bindings.at(row).first >= m_outputsModel->rowCount()) {
            m_bindings[row].first = 0;
            m_bindings[row].second = 0;
            Q_EMIT dataChanged(index(row,1),index(row,2));
        } else if (m_bindings.at(row).second >= m_outputsModel->
                   getSlaveChannelsCount(m_bindings.at(row).first)) {
            m_bindings[row].second = 0;
            Q_EMIT dataChanged(index(row,2),index(row,2));
        }
    }
}

ZMultiOutputsModel::ZMultiOutputsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ZMultiOutputsModel::~ZMultiOutputsModel() = default;

Qt::ItemFlags ZMultiOutputsModel::flags(const QModelIndex &index) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column()>0)
        flags |= Qt::ItemIsEditable;

    return flags;
}

QVariant ZMultiOutputsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
                case 0: return ZMultiDialog::formatOutputName(index.row());
                case 1: return QSL("%1").arg(m_slaveChannels.at(index.row()));
                default: return QVariant();
            }
        case Qt::UserRole: // valid values
            if (index.column()==1) {
                int maxValue = 99;
                return maxValue;
            }
            break;
        default:
            break;
    }

    return QVariant();
}

int ZMultiOutputsModel::rowCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return m_slaveChannels.count();
}

int ZMultiOutputsModel::columnCount(const QModelIndex &parent) const
{
    if (!checkIndex(parent))
        return 0;
    if (parent.isValid())
        return 0;

    return 2;
}

QVariant ZMultiOutputsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal &&
            role == Qt::DisplayRole) {
        switch (section) {
            case 0: return QSL("Slave PCM");
            case 1: return QSL("Channels count");
            default: return QVariant();
        }
    }
    return QVariant();
}

bool ZMultiOutputsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index,CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid))
        return false;

    int row = index.row();
    int col = index.column();

    if (role == Qt::EditRole) {
        if (col == 1) {
            bool ok;
            int outChannels = value.toInt(&ok);
            if (ok) {
                m_slaveChannels[row] = outChannels;
                Q_EMIT dataChanged(index,index);
                Q_EMIT outputChannelsChanged();
            }
            return true;
        }
    }
    return false;
}

bool ZMultiOutputsModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    beginInsertRows(QModelIndex(), row, row+count-1);
    m_slaveChannels.insert(row,count,2); // insert stereo by default for new outputs
    endInsertRows();

    Q_EMIT outputChannelsChanged();
    return true;
}

bool ZMultiOutputsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!checkIndex(parent))
        return false;
    if (parent.isValid())
        return false;

    beginRemoveRows(parent, row, row+count-1);
    m_slaveChannels.remove(row,count);
    endRemoveRows();

    Q_EMIT outputChannelsChanged();
    return true;
}

int ZMultiOutputsModel::getSlaveChannelsCount(int idx) const
{
    return m_slaveChannels.at(idx);
}

void ZMultiOutputsModel::outputsCountChanged(int value)
{
    if (value<rowCount())
        removeRows(value,rowCount()-value);

    if (value>rowCount())
        insertRows(rowCount(),value-rowCount());
}

QVector<int> ZMultiOutputsModel::getSlaveChannels() const
{
    return m_slaveChannels;
}

void ZMultiOutputsModel::setSlaveChannels(const QVector<int> &slaveChannels)
{
    removeRows(0,rowCount());
    beginInsertRows(QModelIndex(),0,slaveChannels.count()-1);
    m_slaveChannels = slaveChannels;
    endInsertRows();
}
