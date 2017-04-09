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

#include "mainwindow.h"
#include "ms735.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QThread>

inline QString tr(const char *str)
{
    return QCoreApplication::translate("main", str);
}

int testKeys(MS735& mice)
{
    QByteArray macro;
    macro.append("\x00\x01", 2);

    int macroIndex = 0;
    for (int i = 4; i < MS735::MouseLeftButton; ++i)
    {
        macro.append(5).append(i).append(0x85).append(i);

        if ((i % 30) == 0)
        {
            mice.setButton(MS735::ButtonIndex(MS735::SideButton5 + macroIndex),
                           ((macroIndex + MS735::MinMacroNum) << 16) | 9);
            mice.setMacro(++macroIndex, macro.append("\x00\x00", 2));
            macro.resize(0);
            macro.append("\x00\x01", 2);
        }
    }

    // Write last macro
    mice.setButton(MS735::ButtonIndex(MS735::SideButton5 + macroIndex),
                   ((macroIndex + MS735::MinMacroNum) << 16) | 9);
    mice.setMacro(++macroIndex, macro.append("\x00\x00", 2));
    return mice.save() ? 0 : 1;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(PRODUCT_NAME);
    QCoreApplication::setApplicationVersion(PRODUCT_VERSION);
    app.setWindowIcon(QIcon(":/app/icon"));

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("HV MS375 configuration application"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption unbrickOption("unbrick", tr("Switch the device to firmware upgrade mode."));
    parser.addOption(unbrickOption);
    QCommandLineOption profileOption(QStringList() << "p" << "profile", tr("Get the active profile."));
    parser.addOption(profileOption);
    QCommandLineOption setProfileOption(QStringList() << "P" << "set-profile", tr("Select the active <profile>."), tr("profile"));
    parser.addOption(setProfileOption);
    QCommandLineOption backupOption(QStringList() << "b" << "backup", tr("Backup NAND data to a <file>."), tr("file"));
    parser.addOption(backupOption);
    QCommandLineOption restoreOption(QStringList() << "r" << "restore", tr("Restore NAND data from a <file>."), tr("file"));
    parser.addOption(restoreOption);
    QCommandLineOption verboseOption(QStringList() << "verbose", tr("Verbose output."));
    parser.addOption(verboseOption);
    QCommandLineOption testKeysOption(QStringList() << "test-keys", tr("Set up test macros and bind them to the side buttons 5-12."));
    parser.addOption(testKeysOption);

    // Process the actual command line arguments given by the user.
    parser.process(app);

    if (!parser.isSet(verboseOption))
    {
        QLoggingCategory::setFilterRules("*.debug=false");
    }

    auto optionsNames = parser.optionNames();
    optionsNames.removeAll("verbose");

    if (optionsNames.isEmpty())
    {
        MainWindow w;
        w.show();
        return app.exec();
    }

    MS735 mice;

    if (parser.isSet(unbrickOption))
    {
        qWarning() << "If nothing happens, try quickly unplug the device and plug it back.";

        for (int i = 0; i < 10000; ++i)
        {
            if (mice.switchToFirmwareUpgradeMode())
            {
                qWarning() << "The device has been successfully switched to bootloader mode.";
                return 0;
            }

            QThread::usleep(1000);
        }

        qWarning() << "Failed to unbrick the device. You have to disassemble it and link RESET pin to the ground";
        return 1;
    }

    // For any other command line option we need the device, so check it in advance.
    if (!mice.blink())
    {
        qWarning() << "The device was not found.";
        return 1;
    }

    if (parser.isSet(backupOption))
    {
        qWarning() << "Reading config from the device...";

        QFile file(parser.value(backupOption));

        if (!file.open(QFile::WriteOnly))
        {
            qWarning() << "Failed to open" << file.fileName() << "for writing.";
            return 2;
        }

        if (!mice.backupConfig(&file))
        {
            qWarning() << "Failed to read the config.";
            return 3;
        }

        qWarning() << "The config has been successfully read from the device and written to" << file.fileName();
        return 0;
    }

    if (parser.isSet(restoreOption))
    {
        qWarning() << "Writing config to the device...";

        QFile file(parser.value(restoreOption));

        if (!file.open(QFile::ReadOnly))
        {
            qWarning() << "Failed to open" << file.fileName() << "for reading.";
            return 2;
        }

        if (!mice.restoreConfig(&file))
        {
            qWarning() << "Failed to write the config.";
            return 3;
        }

        qWarning() << "The config has been successfully read from " << file.fileName() << " and written to the device";
        return 0;
    }

    if (parser.isSet(profileOption))
    {
        qWarning() << mice.profile();
        return 0;
    }

    if (parser.isSet(setProfileOption))
    {
        mice.setProfile(parser.value(setProfileOption).toInt());
        return 0;
    }

    if (parser.isSet(testKeysOption))
    {
        return testKeys(mice);
    }

    // Should never happen.
    qCritical() << "Unhandled options: " << parser.optionNames();
    return 0;
}
