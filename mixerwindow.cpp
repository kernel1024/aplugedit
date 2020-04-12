#include <QtWidgets>
#include "includes/mixerwindow.h"
#include "includes/alsabackend.h"
#include "includes/generic.h"
#include "ui_mixerwindow.h"
#include "ui_mixeritem.h"

ZMixerWindow::ZMixerWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZMixerWindow)
{
    ui->setupUi(this);
    ui->tabWidget->clear();

    connect(gAlsa,&ZAlsaBackend::alsaMixerReconfigured,this,&ZMixerWindow::reloadControls);//,Qt::QueuedConnection);
    connect(gAlsa,&ZAlsaBackend::alsaMixerValueChanged,this,&ZMixerWindow::updateControlsState);//,Qt::QueuedConnection);
    connect(ui->btnReloadAll,&QPushButton::clicked,this,&ZMixerWindow::reloadAllCards);

    reloadAllCards();
}

ZMixerWindow::~ZMixerWindow()
{
    delete ui;
}

void ZMixerWindow::reloadControls(int cardNum)
{
    const auto mixerItems = gAlsa->getMixerControls(cardNum);
    if (cardNum >= m_controls.count())
        m_controls.resize(cardNum+1);

    m_controls[cardNum] = mixerItems;

    const auto cards = gAlsa->cards();
    while (cardNum >= ui->tabWidget->count()) {
        auto wtab = new QWidget();
        ui->tabWidget->addTab(wtab,cards.at(ui->tabWidget->count()).cardName);
    }

    QVector<CMixerItem> enums;
    QVector<CMixerItem> switches;

    QHBoxLayout* boxLayout = new QHBoxLayout();

    for (const auto& item : mixerItems) {
        if (item.type == CMixerItem::itEnumerated) enums.append(item);
        if ((item.type == CMixerItem::itBoolean) && (!item.isRelated)) switches.append(item);
        if ((item.type != CMixerItem::itInteger) && (item.type != CMixerItem::itInteger64)) continue;

        auto witem = new QWidget;
        Ui::ZMixerItem iui;
        iui.setupUi(witem);

        if (item.relatedNameLength>0) {
            iui.label->setText(item.name.left(item.relatedNameLength));
            //iui.slider->setToolTip(item.name);
        } else {
            iui.label->setText(item.name);
        }

        iui.slider->setMinimum(item.valueMin);
        iui.slider->setMaximum(item.valueMax);
        if (item.valueStep>0)
            iui.slider->setSingleStep(item.valueStep);
        iui.slider->setValue(item.values.constFirst());
        iui.slider->setObjectName(QSL("ctl#%1#%2").arg(cardNum).arg(item.numid));

        iui.btnDelete->setVisible(item.isUser);
        iui.btnDelete->setObjectName(QSL("btn#%1#%2").arg(cardNum).arg(item.numid));

        iui.check->setVisible(false);
        for (const auto& ridx : qAsConst(item.related)) {
            const auto ritem = mixerItems.at(ridx);
            if (ritem.type == CMixerItem::itBoolean) {
                iui.check->setVisible(true);
                iui.check->setChecked(ritem.values.constFirst() == 0L);
                iui.check->setObjectName(QSL("ctl#%1#%2").arg(cardNum).arg(ritem.numid));
                //iui.check->setToolTip(ritem.name);
                break;
            }
        }

        connect(iui.slider,&QSlider::valueChanged,this,&ZMixerWindow::volumeChanged);
        connect(iui.check,&QCheckBox::toggled,this,&ZMixerWindow::switchClicked);
        connect(iui.btnDelete,&QPushButton::clicked,this,&ZMixerWindow::deleteClicked);

        addSeparatedWidgetToLayout(boxLayout,witem);
    }

    if (!switches.isEmpty()) {
        auto checkList = new QListWidget();
        for (const auto &item : qAsConst(switches)) {
            auto itm = new QListWidgetItem(item.name);
            itm->setData(Qt::UserRole,item.numid);
            itm->setData(Qt::UserRole+1,cardNum);
            itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
            itm->setCheckState((item.values.constFirst() == 0L) ? Qt::Checked : Qt::Unchecked);
            checkList->addItem(itm);
        }
        connect(checkList,&QListWidget::itemChanged,this,&ZMixerWindow::switchListClicked);
        addSeparatedWidgetToLayout(boxLayout,checkList);
    }

    if (!enums.isEmpty()) {
        auto enumsList = new QWidget();
        auto enumsLayout = new QVBoxLayout();
        for (const auto &item : qAsConst(enums)) {
            auto enFrame = new QWidget();
            auto enLabel = new QLabel(item.name);
            auto enList = new QComboBox();
            enList->addItems(item.labels);
            enList->setCurrentIndex(item.values.constFirst());
            enList->setObjectName(QSL("ctl#%1#%2").arg(cardNum).arg(item.numid));

            auto subLayout = new QVBoxLayout();
            subLayout->setContentsMargins(QMargins());
            subLayout->addWidget(enLabel);
            subLayout->addWidget(enList);
            subLayout->addStretch();
            enFrame->setLayout(subLayout);

            connect(enList,qOverload<int>(&QComboBox::currentIndexChanged),this,&ZMixerWindow::enumClicked);
            addSeparatedWidgetToLayout(enumsLayout,enFrame);
        }
        enumsList->setLayout(enumsLayout);
        addSeparatedWidgetToLayout(boxLayout,enumsList);
    }

    auto wtab = ui->tabWidget->widget(cardNum);
    if ((wtab != nullptr) && (boxLayout->count() > 0)) {
        clearTab(cardNum);
        wtab->setLayout(boxLayout);
    } else {
        boxLayout->deleteLater();
    }
}

void ZMixerWindow::updateControlsState(int cardNum)
{
    // TODO: update controls value, with update interlocks

}

void ZMixerWindow::addSeparatedWidgetToLayout(QLayout *layout, QWidget *itemWidget)
{
    if (layout->count()>0) {
        QFrame* line = new QFrame();
        if (qobject_cast<QVBoxLayout *>(layout) != nullptr) {
            line->setFrameShape(QFrame::HLine);
        } else {
            line->setFrameShape(QFrame::VLine);
        }
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
    }
    layout->addWidget(itemWidget);
}

bool ZMixerWindow::getMixerItemIDs(QWidget *widget, int *card, unsigned int *numid)
{
    static const QStringList validPrefixes({ QSL("ctl"), QSL("btn") });

    const QStringList sl = widget->objectName().split(QChar('#'));
    if ((sl.count() != 3) || (!validPrefixes.contains(sl.constFirst()))) return false;
    bool ok1;
    bool ok2;
    *card = sl.at(1).toInt(&ok1);
    *numid = sl.at(2).toUInt(&ok2);
    return (ok1 && ok2);
}

void ZMixerWindow::clearTab(int cardNum)
{
    const auto wtab = ui->tabWidget->widget(cardNum);
    clearLayout(wtab->layout()); // delete layouts and layouted widgets
    const auto wlist = wtab->findChildren<QWidget *>(); // delete remaining widgets, if any
    for (const auto& w : wlist)
        w->deleteLater();
}

void ZMixerWindow::clearLayout(QLayout *layout)
{
    if (layout == nullptr) return;
    QLayoutItem * item;
    QWidget * widget;
    while ((item = layout->takeAt(0))) {
        clearLayout(item->layout());
        if ((widget = item->widget()) != nullptr)
            widget->deleteLater();
        delete item;
    }
    delete layout;
}

void ZMixerWindow::reloadAllCards()
{
    const auto cards = gAlsa->cards();
    for (int card=0; card<cards.count(); card++)
        reloadControls(card);
}

void ZMixerWindow::volumeChanged(int value)
{
    auto w = qobject_cast<QSlider *>(sender());
    if (w == nullptr) return;

    int card;
    unsigned int numid;
    if (!getMixerItemIDs(w,&card,&numid)) return;

    for (int i=0; i<m_controls.at(card).count(); i++) {
        if (m_controls.at(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.at(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = value;
            gAlsa->setMixerControl(card,m_controls.at(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::switchClicked(bool state)
{
    auto w = qobject_cast<QCheckBox *>(sender());
    if (w == nullptr) return;

    int card;
    unsigned int numid;
    if (!getMixerItemIDs(w,&card,&numid)) return;

    for (int i=0; i<m_controls.at(card).count(); i++) {
        if (m_controls.at(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.at(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = (state ? 0L : 1L);
            gAlsa->setMixerControl(card,m_controls.at(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::switchListClicked(QListWidgetItem *item)
{
    bool ok1;
    bool ok2;
    unsigned int numid = item->data(Qt::UserRole).toUInt(&ok1);
    int card = item->data(Qt::UserRole+1).toInt(&ok2);
    if (!ok1 || !ok2) return;

    for (int i=0; i<m_controls.at(card).count(); i++) {
        if (m_controls.at(card).at(i).numid == numid) {
            bool oldState = (m_controls[card][i].values.constFirst() == 0L);
            bool newState = (item->checkState() == Qt::CheckState::Checked);
            if (oldState != newState) {
                for (int k=0; k<m_controls.at(card).at(i).values.count(); k++)
                    m_controls[card][i].values[k] = (newState ? 0L : 1L);
            }
            gAlsa->setMixerControl(card,m_controls.at(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::enumClicked(int index)
{
    auto w = qobject_cast<QComboBox *>(sender());
    if (w == nullptr) return;

    int card;
    unsigned int numid;
    if (!getMixerItemIDs(w,&card,&numid)) return;

    for (int i=0; i<m_controls.at(card).count(); i++) {
        if (m_controls.at(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.at(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = index;
            gAlsa->setMixerControl(card,m_controls.at(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::deleteClicked()
{
    auto w = qobject_cast<QPushButton *>(sender());
    if (w == nullptr) return;

    int card;
    unsigned int numid;
    if (!getMixerItemIDs(w,&card,&numid)) return;

    for (int i=0; i<m_controls.at(card).count(); i++) {
        if (m_controls.at(card).at(i).numid == numid) {
            gAlsa->deleteMixerControl(card,m_controls.at(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}
