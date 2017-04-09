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

    enum Mode
    {
        // Real modes
        ModeKey = 0,
        ModeButton = 1,
        ModeCommand = 3,
        ModeDpi = 7,
        ModeMacro = 9,
        ModeSequence = 10,
        // Virtual modes
        ModeOff = 0xFF,
        ModeCustom = 0xFE,
    };

    Q_OBJECT

public:
    explicit ButtonEdit(QString labelText, QWidget *parent = 0);

    quint32 value() const;
    void setValue(quint32 value);

signals:

public slots:
    void onModeChanged(int idx);

private slots:

private:
    quint32 extractValue(Mode mode) const;
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

    // DPI (7)
    QComboBox *cbDpi;

    // Macro (9)
    QLabel *labelRepeat;
    QSpinBox *spinMacroIndex;
    QComboBox *cbRepeatMode;

    // Expert mode
    QLineEdit *editCustom;
};

#endif // BUTTONEDIT_H
