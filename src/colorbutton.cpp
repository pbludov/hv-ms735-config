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

#include "colorbutton.h"

#include <QColorDialog>

ColorButton::ColorButton(QWidget *parent)
    : QPushButton(parent)
    , color(-1)
{
    connect(this, SIGNAL(clicked(bool)), this, SLOT(click()));
}

int ColorButton::value() const
{
    return color;
}

void ColorButton::setValue(int value)
{
    QPixmap px(128, 128);
    px.fill(QColor(value));
    setIcon(px);
    color = value;
}

void ColorButton::click()
{
    QColor value = QColorDialog::getColor(QColor::fromRgba(color), parentWidget());
    if (value.isValid())
    {
        setValue(value.rgba() & 0xFFFFFF);
    }
}
