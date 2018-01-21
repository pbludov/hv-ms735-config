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

#ifndef BUTTONEDIT_H
#define BUTTONEDIT_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

class ButtonEdit : public QWidget
{
    Q_PROPERTY(quint32 value READ value WRITE setValue)

    Q_OBJECT

public:
    explicit ButtonEdit(QString labelText, QWidget *parent = 0);

    quint32 value() const;
    void setValue(quint32 value);

    void highlight(bool on);

public slots:
    void onModeChanged(int idx);

private:
    quint32 extractValue(int mode) const;
    void hideWidgets();

    QLabel *label;
    QComboBox *cbMode;

    // Key (0)
    class QxtCheckComboBox *cbModifiers;
    class EnumEdit *editScans[2];

    // Mouse (1), Sequence (10)
    class MouseButtonBox *cbButton;
    QSpinBox *spinCount;
    QSpinBox *spinDelay;

    // Command (3)
    class EnumEdit *editCommand;

    // Profile (7)
    QComboBox *cbProfile;

    // Macro (9)
    QLabel *labelRepeat;
    QSpinBox *spinMacroIndex;
    QComboBox *cbRepeatMode;

    // Expert mode
    QLineEdit *editCustom;
};

#endif // BUTTONEDIT_H
