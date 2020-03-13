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

#ifndef LADSPA_P_H
#define LADSPA_P_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

namespace ZLADSPA {
Q_NAMESPACE
enum Control {
    aacToggle,
    aacInteger,
    aacLinear,
    aacLogarithmic,
    aacFreq
};
Q_ENUM_NS(Control)

enum Policy {
    plNone = 0,
    plDuplicate = 1
};
Q_ENUM_NS(Policy)
}

using CInOutBindings = QVector<QPair<int,QString> >;

class ZLADSPADialog;

class ZLADSPAControlItem
{
public:
    ZLADSPA::Control aatType { ZLADSPA::aacToggle };
    bool aasToggle { false };
    double aasValue { 0.0 };
    int aasFreq { 0 };
    int aasInt { 0 };
    QString portName;
    QWidget* aawControl { nullptr };
    QLayout* aawLayout { nullptr };
    QLabel* aawLabel { nullptr };

    ZLADSPAControlItem() = default;
    explicit ZLADSPAControlItem(QDataStream &s);
    explicit ZLADSPAControlItem(const QJsonValue &json);
    ZLADSPAControlItem(const ZLADSPAControlItem& other) = default;
    ZLADSPAControlItem(const QString &AportName, ZLADSPA::Control AaatType, bool AaasToggle,
                       double AaasValue, QWidget* AaawControl, QLayout* AaawLayout, QLabel* AaawLabel);

    QJsonValue storeToJson() const;
    void destroyControls();
    void disconnectFromControls();

};

QDataStream &operator<<(QDataStream &, const ZLADSPAControlItem* &);
QDataStream &operator>>(QDataStream &, ZLADSPAControlItem* &);

class ZResizableFrame : public QFrame
{
    Q_OBJECT
public:
    QScrollArea* m_scroller;
    ZResizableFrame(QWidget *parent);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
};

class CLADSPAPlugItem
{
public:
    QString plugLabel;
    QString plugID;
    QString plugName;
    QString plugLibrary;
    ZLADSPA::Policy policy { ZLADSPA::Policy::plDuplicate };
    bool usePolicy { false };
    CInOutBindings inputBindings;
    CInOutBindings outputBindings;
    QVector<ZLADSPAControlItem> plugControls;
    CLADSPAPlugItem() = default;
    ~CLADSPAPlugItem() = default;
    CLADSPAPlugItem(const CLADSPAPlugItem& other) = default;
    explicit CLADSPAPlugItem(const QJsonValue &json);
    CLADSPAPlugItem(const QString &AplugLabel,
                    const QString &AplugID,
                    const QString &AplugName,
                    const QString &AplugLibrary,
                    const QVector<ZLADSPAControlItem> &aPlugControls, bool aUsePolicy = false,
                    ZLADSPA::Policy aPolicy = ZLADSPA::Policy::plDuplicate,
                    const CInOutBindings &aInputBindings = CInOutBindings(),
                    const CInOutBindings &aOutputBindings = CInOutBindings());
    QJsonValue storeToJson() const;
    QStringList getBindingsDesc() const;
};

class ZLADSPAPortEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ZLADSPAPortEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class ZLADSPABindingsModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    CInOutBindings m_bindings;
    QStringList m_validPorts;
    ZLADSPADialog* m_mainDialog;
public:
    ZLADSPABindingsModel(ZLADSPADialog *parent);
    ~ZLADSPABindingsModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void setValidPorts(const QStringList &validPorts);
    CInOutBindings getBindings() const;
    void setBindings(const CInOutBindings& bindings);
    void appendRow();
    void deleteRow(const QModelIndex &index);
};

class ZLADSPAListModel : public QAbstractListModel
{
    Q_OBJECT
private:
    QVector<CLADSPAPlugItem> m_items;

    Q_DISABLE_COPY(ZLADSPAListModel)

public:
    ZLADSPAListModel(QObject *parent);
    ~ZLADSPAListModel() override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;

    Qt::DropActions supportedDropActions() const override;

    int getRowIndex(const QModelIndex &index);

    QVector<CLADSPAPlugItem> items() const;
    void setItems(const QVector<CLADSPAPlugItem> &items);
    void addItems(const QVector<CLADSPAPlugItem> &items);
    void insertItems(int row, const QVector<CLADSPAPlugItem> &items);
    void deleteAllItems();
    void addItem(const CLADSPAPlugItem& item);
    void insertItem(int row, const CLADSPAPlugItem &item);
    void setItem(int row, const CLADSPAPlugItem &item);
};

#endif // LADSPA_P_H
