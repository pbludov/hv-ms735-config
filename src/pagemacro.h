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

#ifndef PAGEMACRO_H
#define PAGEMACRO_H

#include "micewidget.h"

namespace Ui
{
class PageMacro;
}
class PageMacro : public MiceWidget
{
    Q_PROPERTY(QByteArray macro READ macro WRITE setMacro)

    Q_OBJECT

public:
    explicit PageMacro(QWidget *parent = 0);
    ~PageMacro();

    bool load(class MS735 *mice);
    void save(class MS735 *mice);

    QByteArray macro() const;
    void setMacro(QByteArray& macro);

public slots:
    void addAction(int idx);
    void selectMacro(class QListWidgetItem *current, class QListWidgetItem *previous);

private:
    Ui::PageMacro *ui;
    class MS735 *mice;
};

#endif // PAGEMACRO_H
