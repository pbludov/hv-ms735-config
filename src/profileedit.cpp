/*
 *      Copyright 2017 Pavel Bludov <pbludov@gmail.com>
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

#include "profileedit.h"
#include "colorbutton.h"
#include "ms735.h"

#include <QCheckBox>
#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QxtCheckComboBox>

ProfileEdit::ProfileEdit(int index, QWidget *parent)
    : MiceWidget(parent)
    , indexValue(index)
{
    const int spacing = 24;
    auto measuredWidth = fontMetrics().width(tr("DPI X    000000"));
    auto layout = new QHBoxLayout;

    btnActive = new QRadioButton(tr("Profile &%1").arg(index + 1));
    layout->addWidget(btnActive);
    layout->addSpacing(spacing);
    connect(btnActive, SIGNAL(clicked(bool)), this, SLOT(onSelectProfile(bool)));

    checkEnabled = new QCheckBox(tr("&Enabled"));
    layout->addWidget(checkEnabled);
    layout->addSpacing(spacing);
    connect(checkEnabled, SIGNAL(toggled(bool)), this, SLOT(onEnableProfile(bool)));

    auto layoutDpi = new QFormLayout;
    layoutDpi->setMargin(0);
    labelDpiX = new QLabel();
    labelDpiX->setMinimumWidth(measuredWidth);
    sliderDpiX = new QSlider(Qt::Horizontal);
    sliderDpiX->setRange(MS735::MinDpi, MS735::MaxDpi);
    labelDpiX->setBuddy(sliderDpiX);
    layoutDpi->addRow(labelDpiX, sliderDpiX);
    connect(sliderDpiX, SIGNAL(valueChanged(int)), this, SLOT(onDpiXChanged(int)));

    labelDpiY = new QLabel();
    labelDpiY->setMinimumWidth(measuredWidth);
    sliderDpiY = new QSlider(Qt::Horizontal);
    sliderDpiY->setRange(MS735::MinDpi, MS735::MaxDpi);
    labelDpiY->setBuddy(sliderDpiY);
    layoutDpi->addRow(labelDpiY, sliderDpiY);
    connect(sliderDpiY, SIGNAL(valueChanged(int)), this, SLOT(onDpiYChanged(int)));

    layout->addLayout(layoutDpi);

    checkSyncDpi = new QCheckBox(tr("&Sync DPI"));
    layout->addWidget(checkSyncDpi);
    connect(checkSyncDpi, SIGNAL(toggled(bool)), this, SLOT(onSyncDpiToggled(bool)));

    layout->addSpacing(spacing);

    auto labelColor = new QLabel(tr("&Color"));
    btnColor = new ColorButton;
    labelColor->setBuddy(btnColor);
    layout->addWidget(labelColor);
    layout->addWidget(btnColor);
    layout->addSpacing(spacing);

    auto labelLeds = new QLabel(tr("&Leds"));
    cbLeds = new QxtCheckComboBox;
    cbLeds->setMinimumWidth(measuredWidth);
    cbLeds->addItem(tr("1"), 1);
    cbLeds->addItem(tr("2"), 2);
    cbLeds->addItem(tr("3"), 4);
    cbLeds->addItem(tr("4"), 8);
    labelLeds->setBuddy(cbLeds);
    layout->addWidget(labelLeds);
    layout->addWidget(cbLeds);

    layout->addStretch();
    setLayout(layout);
}

bool ProfileEdit::load(class MS735 *mice)
{
    auto enabled = mice->profileEnabled(indexValue);
    auto dpiX = mice->profileDpiX(indexValue);
    auto dpiY = mice->profileDpiY(indexValue);
    auto color = mice->profileColor(indexValue);
    auto leds = mice->profileLeds(indexValue);
    if (dpiX < 0 || dpiY < 0 || color < 0 || leds < 0)
        return false;

    onEnableProfile(enabled);
    checkEnabled->setChecked(enabled);
    sliderDpiX->setValue(dpiX);
    sliderDpiY->setValue(dpiY);
    checkSyncDpi->setChecked(dpiX == dpiY);
    btnColor->setValue(color);
    cbLeds->setMask(leds);
    return true;
}

void ProfileEdit::save(class MS735 *mice)
{
    auto leds = cbLeds->mask();
    auto enabled = checkEnabled->isChecked();
    if (enabled && mice->numProfiles() <= indexValue)
    {
        mice->setNumProfiles(indexValue + 1);
    }

    mice->setProfileDpiX(indexValue, sliderDpiX->value());
    mice->setProfileDpiY(indexValue, checkSyncDpi? sliderDpiX->value() : sliderDpiY->value());
    mice->setProfileEnabled(indexValue, enabled);
    mice->setProfileColor(indexValue, btnColor->value());
    mice->setProfileLeds(indexValue, leds);

    if (btnActive->isChecked())
    {
        mice->setProfile(indexValue + 1);
    }
}

bool ProfileEdit::active() const
{
    return btnActive->isChecked();
}

void ProfileEdit::setActive(bool value)
{
    btnActive->setChecked(value);
}

int ProfileEdit::index() const
{
    return indexValue;
}

void ProfileEdit::onDpiXChanged(int value)
{
    labelDpiX->setText(tr("&DPI %1 %2")
                       .arg(checkSyncDpi->isChecked() ? " " : "X")
                       .arg((value + MS735::DpiAdd) * MS735::DpiMul, 6));
    if (checkSyncDpi->isChecked())
        sliderDpiY->setValue(value);
}

void ProfileEdit::onDpiYChanged(int value)
{
    labelDpiY->setText(tr("&DPI Y %1").arg((value + MS735::DpiAdd) * MS735::DpiMul, 6));
    if (checkSyncDpi->isChecked())
        sliderDpiX->setValue(value);
}

void ProfileEdit::onEnableProfile(bool enabled)
{
    if (!enabled)
        btnActive->setChecked(false);

    foreach (auto widget, findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
    {
        if (checkEnabled == widget)
            continue;

        widget->setEnabled(enabled);
    }
}

void ProfileEdit::onSelectProfile(bool)
{
    foreach (auto btn, parentWidget()->findChildren<QRadioButton *>())
    {
        btn->setChecked(btn == btnActive);
    }
}

void ProfileEdit::onSyncDpiToggled(bool sync)
{
    labelDpiY->setVisible(!sync);
    sliderDpiY->setVisible(!sync);

    // Update labelDpiX
    onDpiXChanged(sliderDpiX->value());
}

