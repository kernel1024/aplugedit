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

    ui->statusLabel->clear();

    connect(gAlsa,&ZAlsaBackend::alsaMixerReconfigured,this,&ZMixerWindow::reloadControls);
    connect(gAlsa,&ZAlsaBackend::alsaMixerValueChanged,this,&ZMixerWindow::updateControlsState);
    connect(ui->btnReloadAll,&QPushButton::clicked,this,&ZMixerWindow::reloadAllCards);

    reloadAllCards();
}

ZMixerWindow::~ZMixerWindow()
{
    delete ui;
}

void ZMixerWindow::reloadControls(const QString &ctlName)
{
    // prepare tabs with scrollers for all cards
    const QStringList mixerCtls = gAlsa->getMixerCtls(false);
    while (ui->tabWidget->count() > mixerCtls.count()) {
        int idx = ui->tabWidget->count() - 1;
        QWidget* w = ui->tabWidget->widget(idx);
        ui->tabWidget->removeTab(idx);
        w->deleteLater();
    }
    for (int idx = 0; idx < mixerCtls.count(); idx++) {
        const QString &name = mixerCtls.at(idx);
        const QString displayName = gAlsa->getMixerName(name);
        if (idx+1 > ui->tabWidget->count()) {
            auto *scroller = new QScrollArea();
            scroller->setObjectName(QSL("scroll#%1").arg(name));
            scroller->setWidgetResizable(true);
            ui->tabWidget->addTab(scroller,displayName);
        } else if (ui->tabWidget->tabText(idx) != displayName) {
            ui->tabWidget->setTabText(idx,displayName);
        }
    }

    const auto mixerItems = gAlsa->getMixerControls(ctlName);
    m_controls[ctlName] = mixerItems;

    // prepare control widgets for selected ctl
    QVector<CMixerItem> enums;
    QVector<CMixerItem> switches;
    enums.reserve(mixerItems.count());
    switches.reserve(mixerItems.count());

    // hboxLayout contains all controls for selected ctl
    auto *boxLayout = new QHBoxLayout();

    for (const auto& item : mixerItems) {
        if (item.type == CMixerItem::itEnumerated) enums.append(item);
        if ((item.type == CMixerItem::itBoolean) && (!item.isRelated)) switches.append(item);
        if ((item.type != CMixerItem::itInteger) && (item.type != CMixerItem::itInteger64)) continue;

        auto *witem = new QWidget();
        Ui::ZMixerItem iui;
        iui.setupUi(witem);

        if (item.relatedNameLength>0) {
            iui.label->setText(item.name.left(item.relatedNameLength));
        } else {
            iui.label->setText(item.name);
        }
        iui.slider->setStatusTip(item.name);
        iui.slider->setMinimum(static_cast<int>(item.valueMin));
        iui.slider->setMaximum(static_cast<int>(item.valueMax));
        if (item.valueStep>0)
            iui.slider->setSingleStep(static_cast<int>(item.valueStep));
        iui.slider->setValue(static_cast<int>(item.values.constFirst()));
        iui.slider->setObjectName(QSL("ctl#%1#%2").arg(ctlName).arg(item.numid));
        iui.slider->setProperty("ctl",ctlName);
        iui.slider->setProperty("numid",item.numid);

        iui.btnMenu->setObjectName(QSL("btn#%1#%2").arg(ctlName).arg(item.numid));
        iui.btnMenu->setProperty("ctl",ctlName);
        iui.btnMenu->setProperty("numid",item.numid);

        iui.check->setEnabled(false);
        for (const auto& ridx : qAsConst(item.related)) {
            const auto& ritem = mixerItems.at(ridx);
            if (ritem.type == CMixerItem::itBoolean) {
                iui.check->setEnabled(true);
                iui.check->setChecked(ritem.values.constFirst() == 0L);
                iui.check->setObjectName(QSL("ctl#%1#%2").arg(ctlName).arg(ritem.numid));
                iui.check->setProperty("ctl",ctlName);
                iui.check->setProperty("numid",ritem.numid);
                iui.check->setStatusTip(ritem.name);
                break;
            }
        }

        connect(iui.slider,&QSlider::valueChanged,this,&ZMixerWindow::volumeChanged);
        connect(iui.check,&QCheckBox::toggled,this,&ZMixerWindow::switchClicked);
        connect(iui.btnMenu,&QPushButton::clicked,this,&ZMixerWindow::mixerCtxMenuClicked);

        addSeparatedWidgetToLayout(boxLayout,witem);
    }

    if (!switches.isEmpty()) {
        auto *checkList = new QListWidget();
        checkList->setObjectName(QSL("sw#%1").arg(ctlName));
        for (const auto &item : qAsConst(switches)) {
            auto *itm = new QListWidgetItem(item.name);
            itm->setData(Qt::UserRole,item.numid);
            itm->setData(Qt::UserRole+1,ctlName);
            itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
            itm->setCheckState((item.values.constFirst() == 0L) ? Qt::Checked : Qt::Unchecked);
            itm->setStatusTip(item.name);
            checkList->addItem(itm);
        }
        checkList->setMinimumWidth(checkList->sizeHintForColumn(0) +
                                   checkList->fontMetrics().horizontalAdvance(QChar('X')));
        connect(checkList,&QListWidget::itemChanged,this,&ZMixerWindow::switchListClicked);
        addSeparatedWidgetToLayout(boxLayout,checkList);
    }

    if (!enums.isEmpty()) {
        auto *enumsList = new QWidget();
        auto *enumsLayout = new QVBoxLayout();
        for (const auto &item : qAsConst(enums)) {
            auto *enFrame = new QWidget();
            auto *enLabel = new QLabel(item.name);
            auto *enList = new QComboBox();
            enList->addItems(item.labels);
            enList->setCurrentIndex(static_cast<int>(item.values.constFirst()));
            enList->setObjectName(QSL("ctl#%1#%2").arg(ctlName).arg(item.numid));
            enList->setProperty("ctl",ctlName);
            enList->setProperty("numid",item.numid);
            enList->setStatusTip(item.name);

            auto *subLayout = new QVBoxLayout();
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

    // reallocate scroller widget for ctl, insert hboxLayout to it
    auto *scroller = findChild<QScrollArea *>(QSL("scroll#%1").arg(ctlName));
    if ((scroller != nullptr) && (boxLayout->count() > 0)) {
        if (scroller->widget())
            scroller->takeWidget()->deleteLater();

        auto *wcontainer = new QWidget();
        wcontainer->setObjectName(QSL("card#%1").arg(ctlName));
        wcontainer->setLayout(boxLayout);
        scroller->setWidget(wcontainer);
    } else {
        boxLayout->deleteLater();
    }
}

void ZMixerWindow::reloadControlsQueued(const QString &ctlName)
{
    QMetaObject::invokeMethod(this,[this,ctlName](){
        reloadControls(ctlName);
    },Qt::QueuedConnection);
}

bool ZMixerWindow::event(QEvent *event)
{
    if (event->type() == QEvent::StatusTip) {
        auto *ev = dynamic_cast<QStatusTipEvent *>(event);
        ui->statusLabel->setText(ev->tip());
        return true;
    }

    return QDialog::event(event);
}

void ZMixerWindow::updateControlsState(const QString &ctlName)
{
    auto *tab = findChild<QWidget *>(QSL("card#%1").arg(ctlName));
    if (tab == nullptr) return;

    const auto mixerItems = gAlsa->getMixerControls(ctlName);
    if (mixerItems.count() != m_controls.value(ctlName).count()) {
        reloadControlsQueued(ctlName);
        return;
    }

    for (int i=0; i<mixerItems.count(); i++) {

        const auto& oldItem = m_controls.value(ctlName).at(i);
        const auto& newItem = mixerItems.at(i);
        if ((newItem.numid != oldItem.numid) ||
                (newItem.type != oldItem.type) ||
                (newItem.values.count() != oldItem.values.count())){
            reloadControlsQueued(ctlName);
            return;
        }

        bool equal = true;
        for (int k=0; k<newItem.values.count(); k++) {
            if (newItem.values.at(k) != oldItem.values.at(k)) {
                equal = false;
                break;
            }
        }

        const QString objName(QSL("ctl#%1#%2").arg(ctlName).arg(newItem.numid));

        if (!equal) {
            if ((newItem.type == CMixerItem::itInteger) ||
                    (newItem.type == CMixerItem::itInteger64)) {
                auto *slider = tab->findChild<QSlider *>(objName);
                if ((slider != nullptr) &&
                        (!(slider->isSliderDown())) &&
                        (slider->value() != newItem.values.constFirst())) {
                    slider->blockSignals(true);
                    slider->setValue(static_cast<int>(newItem.values.constFirst()));
                    slider->blockSignals(false);
                }
            } else if (newItem.type == CMixerItem::itBoolean) {
                auto *check = tab->findChild<QCheckBox *>(objName);
                if (check) {
                    if (check->isChecked() != (newItem.values.constFirst() == 0L)) {
                        check->blockSignals(true);
                        check->setChecked(newItem.values.constFirst() == 0L);
                        check->blockSignals(false);
                    }
                } else {
                    auto *checkList = tab->findChild<QListWidget *>(QSL("sw#%1").arg(ctlName));
                    if (checkList) {
                        for (int k=0; k<checkList->count(); k++) {
                            bool ok = 0;
                            auto *clItem = checkList->item(k);
                            if (clItem) {
                                unsigned int checkNumid = clItem->data(Qt::UserRole).toUInt(&ok);
                                const QString checkCtlName = clItem->data(Qt::UserRole + 1).toString();
                                if ((checkCtlName == ctlName) && (checkNumid == newItem.numid)) {
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
                auto *enList = tab->findChild<QComboBox *>(objName);
                if ((enList != nullptr) &&
                        (enList->currentIndex() != newItem.values.constFirst())) {
                    enList->blockSignals(true);
                    enList->setCurrentIndex(static_cast<int>(newItem.values.constFirst()));
                    enList->blockSignals(false);
                }
            }

            m_controls[ctlName][i] = mixerItems.at(i);
        }
    }
}

void ZMixerWindow::addSeparatedWidgetToLayout(QLayout *layout, QWidget *itemWidget)
{
    if (layout->count()>0) {
        auto *line = new QFrame();
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

void ZMixerWindow::reloadAllCards()
{
    gAlsa->reloadGlobalConfig();
    const QStringList &sl = gAlsa->getMixerCtls(true);
    for (const auto &ctlName : sl)
        reloadControls(ctlName);
}

void ZMixerWindow::volumeChanged(int value)
{
    auto *w = qobject_cast<QSlider *>(sender());
    if (w == nullptr) return;

    bool ok = false;
    const QString card = w->property("ctl").toString();
    unsigned int numid = w->property("numid").toUInt(&ok);
    if (!ok) return;

    for (int i=0; i<m_controls.value(card).count(); i++) {
        if (m_controls.value(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.value(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = value;
            gAlsa->setMixerControl(card,m_controls.value(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::switchClicked(bool state)
{
    auto *w = qobject_cast<QCheckBox *>(sender());
    if (w == nullptr) return;

    bool ok = false;
    const QString card = w->property("ctl").toString();
    unsigned int numid = w->property("numid").toUInt(&ok);
    if (!ok) return;

    for (int i=0; i<m_controls.value(card).count(); i++) {
        if (m_controls.value(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.value(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = (state ? 0L : 1L);
            gAlsa->setMixerControl(card,m_controls.value(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::switchListClicked(QListWidgetItem *item)
{
    bool ok = false;
    unsigned int numid = item->data(Qt::UserRole).toUInt(&ok);
    const QString card = item->data(Qt::UserRole+1).toString();
    if (!ok) return;

    for (int i=0; i<m_controls.value(card).count(); i++) {
        if (m_controls.value(card).at(i).numid == numid) {
            bool oldState = (m_controls[card][i].values.constFirst() == 0L);
            bool newState = (item->checkState() == Qt::CheckState::Checked);
            if (oldState != newState) {
                for (int k=0; k<m_controls.value(card).at(i).values.count(); k++)
                    m_controls[card][i].values[k] = (newState ? 0L : 1L);
            }
            gAlsa->setMixerControl(card,m_controls.value(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::enumClicked(int index)
{
    auto *w = qobject_cast<QComboBox *>(sender());
    if (w == nullptr) return;

    bool ok = false;
    const QString card = w->property("ctl").toString();
    unsigned int numid = w->property("numid").toUInt(&ok);
    if (!ok) return;

    for (int i=0; i<m_controls.value(card).count(); i++) {
        if (m_controls.value(card).at(i).numid == numid) {
            for (int k=0; k<m_controls.value(card).at(i).values.count(); k++)
                m_controls[card][i].values[k] = index;
            gAlsa->setMixerControl(card,m_controls.value(card).at(i));
            break;
        }
    }
    if (gAlsa->isWarnings())
        QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
}

void ZMixerWindow::mixerCtxMenuClicked()
{
    auto *w = qobject_cast<QPushButton *>(sender());
    if (w == nullptr) return;

    bool ok = false;
    const QString card = w->property("ctl").toString();
    unsigned int numid = w->property("numid").toUInt(&ok);
    if (!ok) return;

    CMixerItem mxItem;
    for (const auto& itm : m_controls.value(card)) {
        if (itm.numid == numid) {
            mxItem = itm;
            break;
        }
    }
    if (mxItem.isEmpty()) return;

    QMenu menu;
    auto *ac = menu.addAction(tr("Delete control"));
    ac->setEnabled(mxItem.isUser);
    connect(ac,&QAction::triggered,this,[this,card,mxItem](){
        gAlsa->deleteMixerControl(card,mxItem);
        if (gAlsa->isWarnings())
            QMessageBox::warning(this,tr("ALSA mixer"),gAlsa->getAlsaWarnings().join(QChar('\n')));
    });

    menu.exec(QCursor::pos());
}
