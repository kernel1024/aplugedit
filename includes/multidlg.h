#ifndef MULTIDLG_H
#define MULTIDLG_H

#include <QDialog>
#include <QAbstractTableModel>
#include "cpmulti.h"

class ZMultiDialog;
class ZMultiOutputsModel;

class ZMultiBindingsModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QVector<CMultiBinding> m_bindings;
    ZMultiOutputsModel *m_outputsModel;

public:
    explicit ZMultiBindingsModel(QObject *parent, ZMultiOutputsModel *outputsModel);
    ~ZMultiBindingsModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void setBindings(const QVector<CMultiBinding> &getBindings);
    QVector<CMultiBinding> getBindings() const;

public Q_SLOTS:
    void capacityChanged(int value);
    void outputsChanged();

};

class ZMultiOutputsModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    QVector<int> m_slaveChannels;

public:
    ZMultiOutputsModel(QObject *parent);
    ~ZMultiOutputsModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    int getSlaveChannelsCount(int idx) const;

    QVector<int> getSlaveChannels() const;
    void setSlaveChannels(const QVector<int> &slaveChannels);

public Q_SLOTS:
    void outputsCountChanged(int value);

Q_SIGNALS:
    void outputChannelsChanged();

};

namespace Ui {
class ZMultiDialog;
}

class ZMultiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZMultiDialog(QWidget *parent = nullptr);
    ~ZMultiDialog() override;

    void setParams(const QVector<CMultiBinding> &bindings,
                   const QVector<int> &slaveChannels);
    void getParams(QVector<CMultiBinding> &bindings,
                   QVector<int> &slaveChannels);

    static QString formatOutputName(int idx);

private:
    Ui::ZMultiDialog *ui;
    ZMultiBindingsModel *m_bindingsModel;
    ZMultiOutputsModel *m_outputsModel;


};

#endif // MULTIDLG_H
