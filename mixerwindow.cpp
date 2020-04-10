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

    reloadControls();
}

ZMixerWindow::~ZMixerWindow()
{
    delete ui;
}

void ZMixerWindow::reloadControls()
{
    ui->tabWidget->clear();

    const auto cards = gAlsa->cards();

    QVector<CMixerItem> enums;
    QVector<CMixerItem> switches;

    int cardsCount = cards.count();
    for (int card=0; card<cardsCount; card++) {

        QHBoxLayout* boxLayout = new QHBoxLayout();
        const auto mixerItems = gAlsa->getMixerControls(card);
        m_controls.append(mixerItems);

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
            iui.slider->setValue(item.values.first());
            iui.slider->setObjectName(QSL("ctl#%1#%2").arg(card).arg(item.numid));

            iui.btnDelete->setVisible(item.isUser);

            iui.check->setVisible(false);
            for (const auto& ridx : qAsConst(item.related)) {
                const auto ritem = mixerItems.at(ridx);
                if (ritem.type == CMixerItem::itBoolean) {
                    iui.check->setVisible(true);
                    iui.check->setChecked(ritem.values.first() == 0);
                    iui.check->setObjectName(QSL("ctl#%1#%2").arg(card).arg(ritem.numid));
                    //iui.check->setToolTip(ritem.name);
                    break;
                }
            }

            connect(iui.slider,&QSlider::valueChanged,this,&ZMixerWindow::volumeChanged);
            connect(iui.check,&QCheckBox::toggled,this,&ZMixerWindow::switchClicked);

            addMixerItemWidget(boxLayout,witem);
        }

        if (!switches.isEmpty()) {
            auto checkList = new QListWidget();
            for (const auto &item : qAsConst(switches)) {
                auto itm = new QListWidgetItem(item.name);
                itm->setData(Qt::UserRole,item.numid);
                itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
                itm->setCheckState((item.values.first() != 0) ? Qt::Checked : Qt::Unchecked);
                checkList->addItem(itm);
            }
            addMixerItemWidget(boxLayout,checkList);
        }

        if (boxLayout->count()>0) {
            auto wtab = new QWidget();
            ui->tabWidget->addTab(wtab,cards.at(card).cardName);
            wtab->setLayout(boxLayout);
        } else {
            boxLayout->deleteLater();
        }
    }
}

void ZMixerWindow::addMixerItemWidget(QLayout *layout, QWidget *itemWidget)
{
    if (layout->count()>0) {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
    }
    layout->addWidget(itemWidget);
}

void ZMixerWindow::volumeChanged(int value)
{

}

void ZMixerWindow::switchClicked(bool state)
{

}
