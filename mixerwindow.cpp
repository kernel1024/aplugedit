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
        auto scroller = new QScrollArea();
        scroller->setObjectName(QSL("scroll#%1").arg(cardNum));
        scroller->setWidgetResizable(true);
        ui->tabWidget->addTab(scroller,cards.at(ui->tabWidget->count()).cardName);
    }

    QVector<CMixerItem> enums;
    QVector<CMixerItem> switches;
    enums.reserve(mixerItems.count());
    switches.reserve(mixerItems.count());

    auto boxLayout = new QHBoxLayout();

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

        iui.slider->setMinimum(static_cast<int>(item.valueMin));
        iui.slider->setMaximum(static_cast<int>(item.valueMax));
        if (item.valueStep>0)
            iui.slider->setSingleStep(static_cast<int>(item.valueStep));
        iui.slider->setValue(static_cast<int>(item.values.constFirst()));
        iui.slider->setObjectName(QSL("ctl#%1#%2").arg(cardNum).arg(item.numid));

        iui.btnDelete->setVisible(item.isUser);
        iui.btnDelete->setObjectName(QSL("btn#%1#%2").arg(cardNum).arg(item.numid));

        iui.check->setVisible(false);
        for (const auto& ridx : qAsConst(item.related)) {
            const auto& ritem = mixerItems.at(ridx);
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
        // TODO: set minimal width
        checkList->setObjectName(QSL("sw#%1").arg(cardNum));
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
            enList->setCurrentIndex(static_cast<int>(item.values.constFirst()));
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

    auto scroller = findChild<QScrollArea *>(QSL("scroll#%1").arg(cardNum));
    if ((scroller != nullptr) && (boxLayout->count() > 0)) {
        if (scroller->widget())
            scroller->takeWidget()->deleteLater(); // NOTE: check control widgets deallocation with valgrind

        auto wtab = new QWidget();
        wtab->setObjectName(QSL("card#%1").arg(cardNum));
//        clearTab(cardNum);
        wtab->setLayout(boxLayout);

        scroller->setWidget(wtab);
    } else {
        boxLayout->deleteLater();
    }
}

void ZMixerWindow::reloadControlsQueued(int cardNum)
{
    QTimer::singleShot(0,this,[this,cardNum](){
        reloadControls(cardNum);
    });
}

void ZMixerWindow::updateControlsState(int cardNum)
{
    auto tab = findChild<QWidget *>(QSL("card#%1").arg(cardNum));
    if (tab == nullptr) return;

    const auto mixerItems = gAlsa->getMixerControls(cardNum);
    if (mixerItems.count() != m_controls.at(cardNum).count()) {
        reloadControlsQueued(cardNum);
        return;
    }

    for (int i=0; i<mixerItems.count(); i++) {

        const auto& oldItem = m_controls.at(cardNum).at(i);
        const auto& newItem = mixerItems.at(i);
        if ((newItem.numid != oldItem.numid) ||
                (newItem.type != oldItem.type) ||
                (newItem.values.count() != oldItem.values.count())){
            reloadControlsQueued(cardNum);
            return;
        }

        bool equal = true;
        for (int k=0; k<newItem.values.count(); k++) {
            if (newItem.values.at(k) != oldItem.values.at(k)) {
                equal = false;
                break;
            }
        }

        const QString objName(QSL("ctl#%1#%2").arg(cardNum).arg(newItem.numid));

        if (!equal) {
            if ((newItem.type == CMixerItem::itInteger) ||
                    (newItem.type == CMixerItem::itInteger64)) {
                auto slider = tab->findChild<QSlider *>(objName);
                if ((slider != nullptr) &&
                        (!(slider->isSliderDown())) &&
                        (slider->value() != newItem.values.constFirst())) {
                    slider->blockSignals(true);
                    slider->setValue(static_cast<int>(newItem.values.constFirst()));
                    slider->blockSignals(false);
                }
            } else if (newItem.type == CMixerItem::itBoolean) {
                auto check = tab->findChild<QCheckBox *>(objName);
                if (check) {
                    if (check->isChecked() != (newItem.values.constFirst() == 0L)) {
                        check->blockSignals(true);
                        check->setChecked(newItem.values.constFirst() == 0L);
                        check->blockSignals(false);
                    }
                } else {
                    auto checkList = tab->findChild<QListWidget *>(QSL("sw#%1").arg(cardNum));
                    if (checkList) {
                        for (int k=0; k<checkList->count(); k++) {
                            bool ok1;
                            bool ok2;
                            auto clItem = checkList->item(k);
                            if (clItem) {
                                unsigned int checkNumid = clItem->data(Qt::UserRole).toUInt(&ok1);
                                int checkCardNum = clItem->data(Qt::UserRole + 1).toInt(&ok2);
                                if ((checkCardNum == cardNum) && (checkNumid == newItem.numid)) {
                                    bool oldState = (clItem->checkState() == Qt::Checked);
                                    bool newState = (newItem.values.constFirst() == 0L);
                                    if (oldState != newState) {
                                        checkList->blockSignals(true);
                                        clItem->setCheckState(newState ? Qt::Checked : Qt::Unchecked);
                                        checkList->blockSignals(false);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            } else if (newItem.type == CMixerItem::itEnumerated) {
                auto enList = tab->findChild<QComboBox *>(objName);
                if ((enList != nullptr) &&
                        (enList->currentIndex() != newItem.values.constFirst())) {
                    enList->blockSignals(true);
                    enList->setCurrentIndex(static_cast<int>(newItem.values.constFirst()));
                    enList->blockSignals(false);
                }
            }

            m_controls[cardNum][i] = mixerItems.at(i);
        }
    }
}

void ZMixerWindow::addSeparatedWidgetToLayout(QLayout *layout, QWidget *itemWidget)
{
    if (layout->count()>0) {
        auto line = new QFrame();
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
    // TODO: remove this, not usable with scroller
    auto wtab = findChild<QWidget *>(QSL("card#%1").arg(cardNum));
    clearLayout(wtab->layout()); // delete layouts and layouted widgets
    const auto wlist = wtab->findChildren<QWidget *>(); // delete remaining widgets, if any
    for (const auto& w : wlist)
        w->deleteLater();
}

void ZMixerWindow::clearLayout(QLayout *layout)
{
    // TODO: remove this
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
