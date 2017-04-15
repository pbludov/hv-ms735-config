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

#include "pagesensitivity.h"
#include "ui_pagesensitivity.h"
#include "ms735.h"

PageSensitivity::PageSensitivity(QWidget *parent)
    : MiceWidget(parent)
    , ui(new Ui::PageSensitivity)
{
    ui->setupUi(this);

    auto measuredWidth = fontMetrics().width(tr("X axis  XXX%"));
    ui->labelX->setMinimumWidth(measuredWidth);
    ui->labelY->setMinimumWidth(measuredWidth);

    ui->labelReportRate->setMinimumWidth(fontMetrics().width(tr("Every XXX report")));
}

PageSensitivity::~PageSensitivity()
{
    delete ui;
}

bool PageSensitivity::load(MS735 *mice)
{
    auto valueX = mice->sensitivityX();
    auto valueY = mice->sensitivityX();
    auto valueDivider = mice->reportRateDivider();
    if (valueX < 0 || valueY < 0 || valueDivider < 0)
        return false;

    ui->sliderX->setValue(valueX);
    onSensitivityXChanged(valueX);
    ui->sliderY->setValue(valueY);
    onSensitivityYChanged(valueY);
    ui->sliderReportRate->setValue(valueDivider);
    onReportRateChanged(valueDivider);
    return true;
}

void PageSensitivity::save(MS735 *mice)
{
    mice->setSensitivityX(ui->sliderX->value());
    mice->setSensitivityY(ui->sliderY->value());
    mice->setReportRateDivider(ui->sliderReportRate->value());
}

void PageSensitivity::onSensitivityXChanged(int value)
{
    ui->labelX->setText(tr("&X axis %1%").arg(value, 4));
}

void PageSensitivity::onSensitivityYChanged(int value)
{
    ui->labelY->setText(tr("&Y axis %1%").arg(value, 4));
}

void PageSensitivity::onReportRateChanged(int value)
{
    ui->labelReportRate->setText(tr("&Every %1 report").arg(value));
}
