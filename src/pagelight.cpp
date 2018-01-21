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

#include "pagelight.h"
#include "ui_pagelight.h"
#include "colorbutton.h"
#include "ms735.h"

PageLight::PageLight(QWidget *parent)
    : MiceWidget(parent)
    , ui(new Ui::PageLight)
{
    ui->setupUi(this);

    ui->cbType->addItems(QStringList() << tr("Off") << tr("Standard") << tr("Respiration") << tr("Neon"));
    ui->cbSource->addItems(QStringList() << tr("None (white)") << tr("Main color") << tr("Color 1")
                                         << tr("Active profile") << tr("Colors 1..5"));

    for (int i = 0; i < MS735::MaxColors; ++i)
    {
        auto btn = new ColorButton;
        ui->layout->addRow(tr("Color &%1").arg(i + 1), btn);
        buttons.push_back(btn);
    }
}

PageLight::~PageLight()
{
    delete ui;
}

bool PageLight::load(MS735 *mice)
{
    auto type = mice->lightType();
    auto value = mice->lightValue();
    auto source = mice->lightSource();
    if (type < 0 || value < 0 || source < 0)
        return false;

    ui->cbType->setCurrentIndex(type);
    // Brightness in range 0..10, delay in range 0..20, so adjust
    ui->sliderValue->setValue(type == MS735::LightStandard ? value * 2 : value);
    ui->cbSource->setCurrentIndex(source);
    ui->btnColor->setValue(mice->mainColor());

    for (int i = 0; i < MS735::MaxColors; ++i)
    {
        buttons[i]->setValue(mice->color(i));
    }

    return true;
}

void PageLight::save(MS735 *mice)
{
    auto type = ui->cbType->currentIndex();
    auto value = ui->sliderValue->value();
    mice->setLightType(type);
    mice->setLightValue(type == MS735::LightStandard ? value / 2 : value);
    mice->setLightSource(ui->cbSource->currentIndex());
    mice->setFixedColor(ui->btnColor->value());

    for (int i = 0; i < MS735::MaxColors; ++i)
    {
        mice->setColor(i, buttons[i]->value());
    }
}

void PageLight::onLightTypeChanged(int value)
{
    bool extraControls = value != MS735::LightOff && value != MS735::LightNeon;
    foreach (auto widget, findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly))
    {
        if (ui->cbType == widget || ui->labelType == widget)
            continue;

        if (ui->sliderValue == widget || ui->labelValue == widget)
        {
            widget->setVisible(value != MS735::LightOff);
            continue;
        }

        widget->setVisible(extraControls);
    }
    ui->labelValue->setText(value == MS735::LightStandard ? tr("Brightn&ess") : tr("D&elay"));
}
