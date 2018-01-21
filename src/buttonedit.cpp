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

#include "buttonedit.h"
#include "usbscancodeedit.h"
#include "mousebuttonbox.h"
#include "usbcommandedit.h"
#include "ms735.h"

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSpinBox>
#include <QxtCheckComboBox>

static int adjust(int value, int minAllowed, int maxAllowed)
{
    return value >= minAllowed && value <= maxAllowed ? value : minAllowed;
}

ButtonEdit::ButtonEdit(QString labelText, QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout;
    layout->setMargin(4);

    label = new QLabel(labelText);
    auto measuredWidth = fontMetrics().width("12345678901234");
    label->setMinimumWidth(measuredWidth);
    layout->addWidget(label);
    cbMode = new QComboBox();
    cbMode->setEditable(false);
    cbMode->addItem(tr("Key"), MS735::EventKey);
    cbMode->addItem(tr("Button"), MS735::EventButton);
    cbMode->addItem(tr("Command"), MS735::EventCommand);
    cbMode->addItem(tr("Profile"), MS735::EventProfile);
    cbMode->addItem(tr("Macro"), MS735::EventMacro);
    cbMode->addItem(tr("Sequence"), MS735::EventSequence);
    cbMode->addItem(tr("Off"), MS735::EventOff);
    cbMode->addItem(tr("Custom"), MS735::EventCustom);
    layout->addWidget(cbMode);
    label->setBuddy(cbMode);
    connect(cbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));

    //
    // Keyboard key or combo
    //
    cbModifiers = new QxtCheckComboBox;
    cbModifiers->addItem(tr("LCtrl"), 0x01);
    cbModifiers->addItem(tr("LShift"), 0x02);
    cbModifiers->addItem(tr("LAlt"), 0x04);
    cbModifiers->addItem(tr("LSuper"), 0x08);
    cbModifiers->addItem(tr("RCtrl"), 0x10);
    cbModifiers->addItem(tr("RShift"), 0x20);
    cbModifiers->addItem(tr("RAlt"), 0x40);
    cbModifiers->addItem(tr("RSuper"), 0x80);
    cbModifiers->setDefaultText(tr("None"));
    cbModifiers->setMinimumWidth(measuredWidth * 2);
    layout->addWidget(cbModifiers);

    for (size_t i = 0; i < _countof(editScans); ++i)
    {
        editScans[i] = new UsbScanCodeEdit();
        layout->addWidget(editScans[i]);
    }

    //
    // Mouse or sequence
    //
    cbButton = new MouseButtonBox;
    layout->addWidget(cbButton);
    spinCount = new QSpinBox;
    spinCount->setRange(2, 0xFF);
    spinCount->setPrefix(tr("repeat  "));
    spinCount->setSuffix(tr("  times"));
    layout->addWidget(spinCount);
    spinDelay = new QSpinBox;
    spinDelay->setRange(0, 0xFF);
    spinDelay->setPrefix(tr("delay  "));
    spinDelay->setSuffix(tr("  msec"));
    layout->addWidget(spinDelay);

    //
    // Command
    //
    editCommand = new UsbCommandEdit();
    editCommand->setMinimumWidth(measuredWidth * 2);
    layout->addWidget(editCommand);

    //
    // Profile
    //
    cbProfile = new QComboBox;
    cbProfile->addItem(tr("Next"), MS735::NextProfile);
    cbProfile->addItem(tr("Previous"), MS735::PreviousProfile);
    cbProfile->addItem(tr("Cycle"), MS735::CycleProfile);
    layout->addWidget(cbProfile);

    //
    // Macro
    //
    spinMacroIndex = new QSpinBox;
    spinMacroIndex->setRange(MS735::MinMacroNum, MS735::MaxMacroNum);
    layout->addWidget(spinMacroIndex);
    labelRepeat = new QLabel(tr("&repeat"));
    labelRepeat->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    layout->addWidget(labelRepeat);
    cbRepeatMode = new QComboBox;
    cbRepeatMode->addItem(tr("number of times"), MS735::MacroRepeatCount);
    cbRepeatMode->addItem(tr("until next key"), MS735::MacroRepeatUntilNextKey);
    cbRepeatMode->addItem(tr("until released"), MS735::MacroRepeatWhileHold);
    cbRepeatMode->setEditable(false);
    layout->addWidget(cbRepeatMode);
    labelRepeat->setBuddy(cbRepeatMode);

    //
    // Expert mode
    //
    editCustom = new QLineEdit;
    editCustom->setInputMask("HH HH HH HH");
    editCustom->setMaximumWidth(measuredWidth);
    layout->addWidget(editCustom);
    layout->addStretch();
    setLayout(layout);
}

void ButtonEdit::setValue(quint32 value)
{
    quint8 mode = value;
    quint8 arg1 = value >> 8;
    quint8 arg2 = value >> 16;
    quint8 arg3 = value >> 24;
    setUpdatesEnabled(false);
    editCustom->setText(QString("%1 %2 %3 %4")
                            .arg(arg3, 2, 16, QChar('0'))
                            .arg(arg2, 2, 16, QChar('0'))
                            .arg(arg1, 2, 16, QChar('0'))
                            .arg(mode, 2, 16, QChar('0')));

    // Actually, there is no 'Off' mode. It is mode 'key' with scan code 00
    if (value == 0)
    {
        mode = MS735::EventOff;
    }

    // Hide everything, will show some ot them later
    hideWidgets();

    switch (mode)
    {
    case MS735::EventKey:
        cbModifiers->setMask(arg1);
        cbModifiers->setVisible(true);
        editScans[0]->setValue(arg2);
        editScans[0]->setVisible(true);
        editScans[1]->setValue(arg3);
        editScans[1]->setVisible(true);
        break;

    case MS735::EventButton:
        cbButton->setValue(arg2);
        cbButton->setVisible(true);
        break;

    case MS735::EventCommand:
        // Command is 16-bit wide
        editCommand->setValue(0xFFFF & (value >> 16));
        editCommand->setVisible(true);
        break;

    case MS735::EventProfile:
        cbProfile->setCurrentIndex(adjust(arg2, MS735::NextProfile, MS735::CycleProfile) - MS735::NextProfile);
        cbProfile->setVisible(true);
        break;

    case MS735::EventMacro:
        spinMacroIndex->setValue(adjust(arg2, MS735::MinMacroNum, MS735::MaxMacroNum));
        spinMacroIndex->setVisible(true);
        labelRepeat->setVisible(true);
        cbRepeatMode->setCurrentIndex(adjust(arg1, MS735::MacroRepeatCount, MS735::MacroRepeatWhileHold));
        cbRepeatMode->setVisible(true);
        break;

    case MS735::EventSequence:
        cbButton->setCurrentIndex(
            adjust(arg1, MS735::MouseLeftButton, MS735::WheelDownButton) - MS735::MouseLeftButton);
        cbButton->setVisible(true);
        spinCount->setValue(arg3);
        spinCount->setVisible(true);
        spinDelay->setValue(arg2);
        spinDelay->setVisible(true);
        break;

    case MS735::EventOff:
        // Everything is already hidden, so nothing to do.
        break;

    default:
        mode = MS735::EventCustom;
        editCustom->setVisible(true);
        break;
    }

    auto block = cbMode->blockSignals(true);
    cbMode->setCurrentIndex(cbMode->findData(mode));
    cbMode->blockSignals(block);
    setUpdatesEnabled(true);
}

void ButtonEdit::highlight(bool on)
{
    label->setStyleSheet(on ? QString("color:HighlightText;font-weight:bold") : QString());
}

quint32 ButtonEdit::value() const
{
    return extractValue(cbMode->currentData().toUInt());
}

quint32 ButtonEdit::extractValue(int mode) const
{
    quint8 arg1 = 0, arg2 = 0, arg3 = 0;

    switch (mode)
    {
    case MS735::EventKey:
        arg1 = cbModifiers->mask();
        arg2 = editScans[0]->value();
        arg3 = editScans[1]->value();
        break;

    case MS735::EventButton:
        arg2 = cbButton->value();
        break;

    case MS735::EventCommand:
    {
        int value = editCommand->value();
        arg2 = 0xFF & value;
        arg3 = 0xFF & (value >> 8);
    }
    break;

    case MS735::EventProfile:
        arg2 = cbProfile->currentIndex() + MS735::NextProfile;
        break;

    case MS735::EventMacro:
        arg2 = spinMacroIndex->value();
        arg1 = cbRepeatMode->currentIndex();
        break;

    case MS735::EventSequence:
        arg1 = cbButton->currentIndex() + MS735::MouseLeftButton;
        arg3 = spinCount->value();
        arg2 = spinDelay->value();
        break;

    case MS735::EventOff:
        return 0;

    case MS735::EventCustom:
        return editCustom->text().replace(" ", "").toUInt(nullptr, 16);

    default:
        Q_ASSERT(!"Unhandled mode");
        break;
    }

    return (arg3 << 24) | (arg2 << 16) | (arg1 << 8) | mode;
}

void ButtonEdit::onModeChanged(int idx)
{
    setUpdatesEnabled(false);
    auto newMode = cbMode->itemData(idx).toUInt();
    auto oldMode = editCustom->text().right(2).toUInt(nullptr, 16);
    quint32 value = extractValue(oldMode) & ~0xFF;

    if (newMode == MS735::EventKey && value == 0)
    {
        // Conversion from OFF to KEY
        value = 0x2C << 16;
    }

    hideWidgets();

    if (newMode == MS735::EventCustom)
    {
        editCustom->setText(QString("%1 %2 %3 %4")
                                .arg(0xFF & value >> 24, 2, 16, QChar('0'))
                                .arg(0xFF & value >> 16, 2, 16, QChar('0'))
                                .arg(0xFF & value >> 8, 2, 16, QChar('0'))
                                .arg(oldMode, 2, 16, QChar('0')));
        editCustom->show();
    }
    else if (newMode != MS735::EventOff)
    {
        setValue(value | newMode);
    }

    setUpdatesEnabled(true);
}

void ButtonEdit::hideWidgets()
{
    foreach (auto widget, findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
    {
        if (widget == label || widget == cbMode)
            continue;

        widget->hide();
    }
}
