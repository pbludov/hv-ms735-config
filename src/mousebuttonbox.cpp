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

#include "mousebuttonbox.h"
#include "ms735.h"

MouseButtonBox::MouseButtonBox(QWidget *parent)
    : QComboBox(parent)
{
    addItem(tr("Primary"), MS735::MouseLeftButton);
    addItem(tr("Secondary"), MS735::MouseRightButton);
    addItem(tr("Third"), MS735::MouseMiddleButton);
    addItem(tr("Backward"), MS735::MouseBackButton);
    addItem(tr("Forward"), MS735::MouseForwardButton);
    addItem(tr("Scroll Left"), MS735::WheelLeftButton);
    addItem(tr("Scroll Right"), MS735::WheelRightButton);
    addItem(tr("Scroll Up"), MS735::WheelUpButton);
    addItem(tr("Scroll Down"), MS735::WheelDownButton);

    setEditable(false);
}

int MouseButtonBox::value() const
{
    return currentData().toInt();
}

void MouseButtonBox::setValue(int value)
{
    setCurrentIndex(findData(value));
}
