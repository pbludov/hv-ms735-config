/*
 *      Copyright 2017-2018 Pavel Bludov <pbludov@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ms735.h"

#include "buttonedit.h"
#include "profileedit.h"
#include "pagelight.h"
#include "pagemacro.h"
#include "pagesensitivity.h"

#include <qxtglobal.h>
#include <QCloseEvent>
#include <QMessageBox>

static void initAction(QAction *action, QStyle::StandardPixmap icon, QKeySequence::StandardKey key)
{
    action->setIcon(qApp->style()->standardIcon(icon));
    action->setShortcut(key);
    action->setToolTip(action->shortcut().toString(QKeySequence::NativeText));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mice(new MS735(this))
{
    ui->setupUi(this);
    // The Designer really lacs this functionality
    initAction(ui->actionExit, QStyle::SP_DialogCloseButton, QKeySequence::Quit);
    initAction(ui->actionSave, QStyle::SP_DialogSaveButton, QKeySequence::Save);

    ui->labelText->setText(ui->labelText->text().arg(PRODUCT_VERSION).arg(__DATE__));
    connect(mice, SIGNAL(connectChanged(bool)), this, SLOT(onMiceConnected(bool)));
    connect(mice, SIGNAL(buttonsPressed(int)), this, SLOT(onButtonsPressed(int)));

    // Check the device availability
    onMiceConnected(mice->ping());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMice()
{
    foreach (auto edit, findChildren<ButtonEdit *>())
    {
        mice->setButton((MS735::ButtonIndex)edit->property("ButtonIndex").toInt(), edit->value());
        mice->setButton((MS735::ButtonIndex)edit->property("ButtonIndex").toInt(), edit->value());
    }

    foreach (auto widget, findChildren<MiceWidget *>())
    {
        widget->save(mice);
    }
}

void MainWindow::onSave()
{
    updateMice();

    if (!mice->blink() || !mice->save())
    {
        QMessageBox::warning(this, windowTitle(), tr("Failed to save"));
    }
}

void MainWindow::onProfileChanged(int, int profile)
{
    foreach (auto edit, findChildren<ProfileEdit *>())
    {
        edit->setActive(edit->index() == profile - 1);
    }
}

void MainWindow::onButtonsPressed(int mask)
{
    foreach (auto edit, findChildren<ButtonEdit *>())
    {
        auto index = (MS735::ButtonIndex)edit->property("ButtonIndex").toInt();
        edit->highlight(mask & (1 << index));
    }
}

void MainWindow::onMiceConnected(bool connected)
{
    auto aboutIndex = ui->tabWidget->indexOf(ui->pageAbout);
    if (!connected)
        ui->tabWidget->setCurrentIndex(aboutIndex);

    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        if (aboutIndex == i)
            continue;
        ui->tabWidget->setTabEnabled(i, connected);
    }

    ui->actionSave->setEnabled(connected);
}

std::pair<QString, MS735::ButtonIndex> mainButtons[] =
{
    {QCoreApplication::translate("button", "Button &Left"),  MS735::ButtonLeft},
    {QCoreApplication::translate("button", "Button &Right"), MS735::ButtonRight},
    {QCoreApplication::translate("button", "Button &Up"),    MS735::ButtonUp},
    {QCoreApplication::translate("button", "Button &Down"),  MS735::ButtonDown},
    {QCoreApplication::translate("button", "&Wheel Left"),   MS735::WheelLeft},
    {QCoreApplication::translate("button", "W&heel Right"),  MS735::WheelRight},
    {QCoreApplication::translate("button", "Wh&eel Up"),     MS735::WheelUp},
    {QCoreApplication::translate("button", "Whee&l Down"),   MS735::WheelDown},
    {QCoreApplication::translate("button", "Wheel &Click"),  MS735::WheelCick},
}, sideButtons[] =
{
    {QCoreApplication::translate("button", "Side Button &1"),  MS735::SideButton1},
    {QCoreApplication::translate("button", "Side Button &2"),  MS735::SideButton2},
    {QCoreApplication::translate("button", "Side Button &3"),  MS735::SideButton3},
    {QCoreApplication::translate("button", "Side Button &4"),  MS735::SideButton4},
    {QCoreApplication::translate("button", "Side Button &5"),  MS735::SideButton5},
    {QCoreApplication::translate("button", "Side Button &6"),  MS735::SideButton6},
    {QCoreApplication::translate("button", "Side Button &7"),  MS735::SideButton7},
    {QCoreApplication::translate("button", "Side Button &8"),  MS735::SideButton8},
    {QCoreApplication::translate("button", "Side Button &9"),  MS735::SideButton9},
    {QCoreApplication::translate("button", "Side Button 1&0"), MS735::SideButton10},
    {QCoreApplication::translate("button", "Side Button 1&1"), MS735::SideButton11},
    {QCoreApplication::translate("button", "Side Button 1&2"), MS735::SideButton12},
};

static bool prepareButtonsPage(
    QWidget *parent, MS735 *mice, const std::pair<QString, MS735::ButtonIndex> *buttons, size_t numButtons)
{
    auto layout = new QVBoxLayout;

    for (size_t i = 0; i < numButtons; ++i)
    {
        auto value = mice->button(buttons[i].second);
        if (value == -1)
        {
            delete layout;
            return false;
        }
        auto edit = new ButtonEdit(buttons[i].first);
        edit->setProperty("ButtonIndex", buttons[i].second);
        edit->setValue(value);
        layout->addWidget(edit);
    }

    layout->addStretch();
    parent->setLayout(layout);
    return true;
}

static bool prepareProfilesPage(QWidget *parent, MS735 *mice)
{
    int activeProfile = mice->profile() - 1;
    if (activeProfile < 0)
        return false;

    auto layout = new QVBoxLayout;
    layout->setMargin(0);

    for (int i = 0; i < MS735::MaxProfiles; ++i)
    {
        auto edit = new ProfileEdit(i);
        if (!edit->load(mice))
        {
            delete edit;
            delete layout;
            return false;
        }

        edit->setActive(i == activeProfile);
        layout->addWidget(edit);
    }

    layout->addStretch();
    parent->setLayout(layout);
    return true;
}

bool MainWindow::initPage(QWidget *parent, MiceWidget *page)
{
    if (!page->load(mice))
    {
        delete page;
        return false;
    }

    auto layout = new QVBoxLayout;
    parent->setLayout(layout);
    layout->addWidget(page);
    return true;
}

void MainWindow::onPreparePage(int idx)
{
    auto page = ui->tabWidget->widget(idx);

    if (page->layout())
    {
        // Already prepared
        return;
    }

    bool ok = true;
    if (page == ui->pageMainButtons)
        ok = prepareButtonsPage(page, mice, mainButtons, _countof(mainButtons));
    else if (page == ui->pageSideButtons)
        ok = prepareButtonsPage(page, mice, sideButtons, _countof(sideButtons));
    else if (page == ui->pageMacros)
        ok = initPage(page, new PageMacro());
    else if (page == ui->pageSensitivity)
        ok = initPage(page, new PageSensitivity());
    else if (page == ui->pageProfiles)
    {
        ok = prepareProfilesPage(page, mice);
        connect(mice, SIGNAL(profileChanged(int,int)), this, SLOT(onProfileChanged(int,int)));
    }
    else if (page == ui->pageLight)
        ok = initPage(page, new PageLight());

    if (!ok)
    {
        QMessageBox::warning(this, windowTitle(), tr("Failed to read mouse NAND"));
    }
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    updateMice();

    if (mice->unsavedChanges()
        && QMessageBox::question(this, windowTitle(), tr("You have unsaved changes.\nSave them now?"))
               == QMessageBox::Yes)
    {
        if (!mice->blink() || !mice->save())
        {
            QMessageBox::warning(this, windowTitle(), tr("Failed to save"));
            evt->ignore();
            return;
        }
    }

    QMainWindow::closeEvent(evt);
}
