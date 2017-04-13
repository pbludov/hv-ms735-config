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

#ifndef MS735_H
#define MS735_H

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(UsbIo)

class MS735 : public QObject
{
    Q_PROPERTY(int mainColor READ mainColor WRITE setFixedColor)
    Q_PROPERTY(int lightSource READ lightSource WRITE setLightSource)
    Q_PROPERTY(int lightType READ lightType WRITE setLightType)
    Q_PROPERTY(int lightValue READ lightValue WRITE setLightValue)
    Q_PROPERTY(int numProfiles READ numProfiles WRITE setNumProfiles)
    Q_PROPERTY(int profile READ profile WRITE setProfile)
    Q_PROPERTY(int sensitivityX READ sensitivityX WRITE setSensitivityX)
    Q_PROPERTY(int sensitivityY READ sensitivityY WRITE setSensitivityY)
    Q_PROPERTY(bool unsavedChanges READ unsavedChanges)

    Q_OBJECT

    enum Command
    {
        CmdEventMask = 1,
        CmdBlink = 2,
        CmdUnknown = 3,
        CmdProfile = 4,
        CmdFirmwareMode = 0x0A,
        CmdControl = 0x0C,
        CmdButtons = 0x0D,
        CmdMacro = 0x0F,
        CmdFlagGet = 0x80,
        CmdGetProfile = CmdProfile | CmdFlagGet,
    };

    enum RegisterOffset
    {
        // 8E 00 3F(01) 00 42 03 FE 0F
        // 8 bytes zero
        LedsOffset = 16,   // 8 bytes
        ColorsOffset = 24, // 3*5 bytes
        // 9 bytes unknown, possible 3 more colors
        // 16 bytes unknown
        // 0F, 04, 0A, 0A, 19, 19
        NumProfilesOffset = 70,
        LightTypeOffset,
        LightValueOffset,
        LightSourceOffset,
        SensitivityXOffset,
        SensitivityYOffset,
        // 01 C0 F0 03 01 01 64 00
        DpiYOffset = 84, // 8 bytes
        DpiXOffset = 92, // 8 bytes
        ProfilesMaskOffset = 100,
        MainColorOffset,           // 3 bytes
        ProfileColorsOffset = 104, // 3*8 bytes
    };

public:
    enum Constants
    {
        MinMacroNum = 1,
        MaxMacroNum = 100,
        MaxColors = 5,
        MaxProfiles = 8,
        MinDpi = 0,
        MaxDpi = 119,
        MaxLeds = 4,
        DpiMul = 100,
        DpiAdd = 1,
    };

    enum ButtonIndex
    {
        // Main buttons
        ButtonLeft,
        ButtonRight,
        WheelCick,
        ButtonUp,
        ButtonDown,
        WheelLeft,
        WheelRight,
        // Side Buttons, 1st row
        SideButton1 = 8,
        SideButton2,
        SideButton3,
        SideButton4,
        // Wheel may be also configured
        WheelUp = 14,
        WheelDown,
        // Side Buttons, 2nd & 3rd rows
        SideButton5,
        SideButton6,
        SideButton7,
        SideButton8,
        SideButton9,
        SideButton10,
        SideButton11,
        SideButton12,
    };

    enum MouseButton
    {
        MouseLeftButton = 0xF0,
        MouseRightButton,
        MouseMiddleButton,
        MouseBackButton,
        MouseForwardButton,
        WheelLeftButton,
        WheelRightButton,
        WheelUpButton,
        WheelDownButton,
    };

    enum LightType
    {
        LightOff,
        LightStandard,
        LightRespiration,
        LightNeon,
    };

    enum LightSource
    {
        LightSourceNone,
        LightSourceMainColor,
        LightSourceColor1,
        LightSourceActiveProfile,
        LightSourceColors1_5
    };

    enum DpiCommand
    {
        DpiNext = 1,
        DpiPrevious,
        // Same as next, but wpars at maximum
        DpiCycle,
    };

    enum MacroRepeatMode
    {
        MacroRepeatCount,
        MacroRepeatUntilNextKey,
        MacroRepeatWhileHold,
    };

    explicit MS735(QObject *parent = 0);
    ~MS735();

    int profile();
    void setProfile(int value);

    int numProfiles();
    void setNumProfiles(int value);

    int sensitivityX();
    void setSensitivityX(int value);

    int sensitivityY();
    void setSensitivityY(int value);

    int lightType();
    void setLightType(int value);

    int lightValue();
    void setLightValue(int value);

    int lightSource();
    void setLightSource(int value);

    int mainColor();
    void setFixedColor(int value);

    bool unsavedChanges();
    bool save();

    int color(int index);
    void setColor(int index, int value);

    int button(ButtonIndex btn);
    void setButton(ButtonIndex btn, int value);

    bool profileEnabled(int profile);
    void setProfileEnabled(int profile, bool value);

    int profileDpiX(int profile);
    void setProfileDpiX(int profile, int value);

    int profileDpiY(int profile);
    void setProfileDpiY(int profile, int value);

    int profileColor(int profile);
    void setProfileColor(int profile, int value);

    int profileLeds(int profile);
    void setProfileLeds(int profile, int value);

    QByteArray macro(int index);
    void setMacro(int index, const QByteArray &value);

    bool blink();
    bool backupConfig(class QIODevice *storage);
    bool restoreConfig(class QIODevice *storage);
    bool switchToFirmwareUpgradeMode();

signals:
    void connectChanged(bool connected);

private slots:
    void deviceArrival(const QString &path);
    void deviceRemove();

private:
    QByteArray report(Command b1, char b2 = 0, char b3 = 0, char b4 = 0, char b5 = 0, char b6 = 0, char b7 = 0);
    char *readPage(Command page, int idx = 0);
    bool writePage(const char *data, Command page, int idx = 0);

    int readByte(Command page, RegisterOffset offset);
    void writeByte(Command page, RegisterOffset offset, int value);

    int readColor(RegisterOffset offset, int index);
    void writeColor(RegisterOffset offset, int index, int value);

    class QHIDDevice *device;
    class QHIDMonitor *monitor;

    std::map<int, char *> cache;
    std::map<int, bool> dirtyPages;
};

#endif // MS735_H
