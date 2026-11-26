/*
 * Copyright (C) 2026 Contributeurs ClassBoard
 *
 * This file is part of ClassBoard, a fork of OpenBoard.
 *
 * ClassBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * ClassBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ClassBoard. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ClassBoardProfile.h"

#include "ClassBoardDisciplines.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

namespace
{
    QString sProfileFileOverride;

    const char* kFirstRunCompleted = "General/FirstRunCompleted";
    const char* kDisciplines       = "General/Disciplines";
    const char* kTheme             = "Appearance/Theme";
    const char* kToolbarScale      = "Appearance/ToolbarScale";
    const char* kShowToolLabels    = "Appearance/ShowToolLabels";
    const char* kLanguage          = "General/Language";
    const char* kFavorites         = "Favorites/Custom";
    const char* kShowFavoritesBar  = "Favorites/ShowBar";
}

bool ClassBoardProfile::isPortable()
{
    if (qEnvironmentVariableIntValue("CLASSBOARD_PORTABLE") == 1)
        return true;

    const QString appDir = QCoreApplication::applicationDirPath();
    return !appDir.isEmpty() && QFile::exists(appDir + "/portable.txt");
}

QString ClassBoardProfile::portableDataDirectory()
{
    return QCoreApplication::applicationDirPath() + "/settings";
}

QString ClassBoardProfile::profileDirectory()
{
    if (isPortable())
        return portableDataDirectory();

    // AppDataLocation = profil itinérant sous Windows (%APPDATA%), afin que
    // les choix de l'enseignant le suivent d'un poste à l'autre.
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Même convention qu'OpenBoard : on retire le niveau "organisation".
    const QString organization = QCoreApplication::organizationName();
    if (!organization.isEmpty())
        dir.replace(organization + "/", "");

    if (dir.isEmpty())
        dir = QDir::homePath() + "/.classboard";

    return dir;
}

QString ClassBoardProfile::profileFile()
{
    if (!sProfileFileOverride.isEmpty())
        return sProfileFileOverride;

    return profileDirectory() + "/ClassBoardProfile.ini";
}

void ClassBoardProfile::setProfileFileOverride(const QString& path)
{
    sProfileFileOverride = path;
}

QVariant ClassBoardProfile::value(const QString& key, const QVariant& defaultValue)
{
    QSettings settings(profileFile(), QSettings::IniFormat);
    return settings.value(key, defaultValue);
}

void ClassBoardProfile::setValue(const QString& key, const QVariant& value)
{
    QDir().mkpath(QFileInfo(profileFile()).absolutePath());
    QSettings settings(profileFile(), QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
}

bool ClassBoardProfile::isFirstRunCompleted()
{
    return value(kFirstRunCompleted, false).toBool();
}

void ClassBoardProfile::setFirstRunCompleted(bool completed)
{
    setValue(kFirstRunCompleted, completed);
}

QStringList ClassBoardProfile::disciplines()
{
    return ClassBoardDisciplines::validIds(value(kDisciplines).toStringList());
}

void ClassBoardProfile::setDisciplines(const QStringList& ids)
{
    setValue(kDisciplines, ClassBoardDisciplines::validIds(ids));
}

ClassBoardProfile::Theme ClassBoardProfile::theme()
{
    return themeFromString(value(kTheme, "light").toString());
}

void ClassBoardProfile::setTheme(Theme theme)
{
    setValue(kTheme, themeToString(theme));
}

QString ClassBoardProfile::themeToString(Theme theme)
{
    switch (theme)
    {
    case Theme::Dark:
        return "dark";
    case Theme::System:
        return "system";
    case Theme::Light:
    default:
        return "light";
    }
}

ClassBoardProfile::Theme ClassBoardProfile::themeFromString(const QString& value)
{
    const QString v = value.trimmed().toLower();

    if (v == "dark")
        return Theme::Dark;

    if (v == "system")
        return Theme::System;

    return Theme::Light;
}

int ClassBoardProfile::toolbarScale()
{
    const int scale = value(kToolbarScale, 125).toInt();
    return qBound(100, scale, 150);
}

void ClassBoardProfile::setToolbarScale(int percent)
{
    setValue(kToolbarScale, qBound(100, percent, 150));
}

bool ClassBoardProfile::showToolLabels()
{
    return value(kShowToolLabels, true).toBool();
}

void ClassBoardProfile::setShowToolLabels(bool show)
{
    setValue(kShowToolLabels, show);
}

QString ClassBoardProfile::interfaceLanguage()
{
    return value(kLanguage).toString();
}

void ClassBoardProfile::setInterfaceLanguage(const QString& language)
{
    setValue(kLanguage, language);
}

QStringList ClassBoardProfile::customFavorites()
{
    return value(kFavorites).toStringList();
}

void ClassBoardProfile::setCustomFavorites(const QStringList& favorites)
{
    setValue(kFavorites, favorites);
}

QStringList ClassBoardProfile::effectiveFavorites()
{
    const QStringList custom = customFavorites();

    if (!custom.isEmpty())
        return custom;

    return ClassBoardDisciplines::favoritesFor(disciplines());
}

bool ClassBoardProfile::showFavoritesBar()
{
    return value(kShowFavoritesBar, true).toBool();
}

void ClassBoardProfile::setShowFavoritesBar(bool show)
{
    setValue(kShowFavoritesBar, show);
}

void ClassBoardProfile::reset()
{
    QSettings settings(profileFile(), QSettings::IniFormat);
    settings.clear();
    settings.sync();
}
