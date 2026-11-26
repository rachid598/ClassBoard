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

#include "ClassBoardTheme.h"

#include "ClassBoardProfile.h"

#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QStyleFactory>
#include <QStyleHints>
#include <QSvgRenderer>

bool ClassBoardTheme::isDark()
{
    switch (ClassBoardProfile::theme())
    {
    case ClassBoardProfile::Theme::Dark:
        return true;
    case ClassBoardProfile::Theme::System:
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
#else
        return false;
#endif
    case ClassBoardProfile::Theme::Light:
    default:
        return false;
    }
}

ClassBoardTheme::Colors ClassBoardTheme::colors()
{
    Colors c;

    if (isDark())
    {
        c.window        = QColor("#1C1C1C");
        c.surface       = QColor("#2B2B2B");
        c.surfaceAlt    = QColor("#363636");
        c.border        = QColor("#3D3D3D");
        c.text          = QColor("#F2F2F2");
        c.textSecondary = QColor("#B5B5B5");
        c.accent        = QColor("#6CB8FF");
        c.accentText    = QColor("#0B2540");
        c.accentSoft    = QColor(108, 184, 255, 46);
        c.danger        = QColor("#FF8A80");
    }
    else
    {
        c.window        = QColor("#F3F3F3");
        c.surface       = QColor("#FFFFFF");
        c.surfaceAlt    = QColor("#F1F3F6");
        c.border        = QColor("#E1E4E8");
        c.text          = QColor("#1A1A1A");
        c.textSecondary = QColor("#5C5C5C");
        c.accent        = QColor("#005FB8");
        c.accentText    = QColor("#FFFFFF");
        c.accentSoft    = QColor("#E5EFFB");
        c.danger        = QColor("#B3261E");
    }

    return c;
}

QString ClassBoardTheme::colorName(const QColor& color)
{
    if (color.alpha() == 255)
        return color.name();

    return QString("rgba(%1, %2, %3, %4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
}

void ClassBoardTheme::apply()
{
    const bool dark = isDark();
    const Colors c = colors();

    QApplication::setStyle(QStyleFactory::create("fusion"));

    QPalette palette = QApplication::style()->standardPalette();

    if (dark)
    {
        palette.setColor(QPalette::Window, c.surface);
        palette.setColor(QPalette::WindowText, c.text);
        palette.setColor(QPalette::Base, c.window);
        palette.setColor(QPalette::AlternateBase, c.surfaceAlt);
        palette.setColor(QPalette::ToolTipBase, c.surfaceAlt);
        palette.setColor(QPalette::ToolTipText, c.text);
        palette.setColor(QPalette::Text, c.text);
        palette.setColor(QPalette::Button, c.surfaceAlt);
        palette.setColor(QPalette::ButtonText, c.text);
        palette.setColor(QPalette::BrightText, Qt::white);
        palette.setColor(QPalette::PlaceholderText, c.textSecondary);
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#7A7A7A"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#7A7A7A"));
        palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#7A7A7A"));
    }

    palette.setColor(QPalette::Highlight, c.accent);
    palette.setColor(QPalette::HighlightedText, c.accentText);
    palette.setColor(QPalette::Link, c.accent);
    QApplication::setPalette(palette);

    // Palettes flottantes du moteur OpenBoard (barre d'outils, couleurs...)
    UBSettings::paletteColor = dark ? QColor(43, 43, 43, 240) : QColor(255, 255, 255, 240);
    UBSettings::opaquePaletteColor = dark ? QColor(43, 43, 43, 250) : QColor(250, 250, 250, 250);

    // Feuille de style globale
    const QString etc = UBPlatformUtils::applicationEtcDirectory();
    QString css = UBFileSystemUtils::readTextFile(etc + "/ClassBoard.css");

    if (dark)
        css += "\n" + UBFileSystemUtils::readTextFile(etc + "/ClassBoard-dark.css");

    qApp->setStyleSheet(css);
}

QSize ClassBoardTheme::scaledIconSize(int baseSize)
{
    const int size = qRound(baseSize * ClassBoardProfile::toolbarScale() / 100.0);
    return QSize(size, size);
}

QIcon ClassBoardTheme::tintedIcon(const QString& svgResource, const QColor& color, int size)
{
    QFile file(svgResource);

    if (!file.open(QIODevice::ReadOnly))
        return QIcon();

    QByteArray svg = file.readAll();
    svg.replace("currentColor", color.name().toLatin1());

    QSvgRenderer renderer(svg);
    QIcon icon;

    for (int scale : {1, 2})
    {
        QPixmap pixmap(size * scale, size * scale);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        renderer.render(&painter);
        painter.end();
        pixmap.setDevicePixelRatio(scale);
        icon.addPixmap(pixmap);
    }

    return icon;
}

QString ClassBoardTheme::dialogStyleSheet()
{
    const Colors c = colors();

    QString css = QStringLiteral(R"(
        QDialog#ClassBoardSetupDialog { background: %1; }
        QWidget#cbCard { background: %2; border: 1px solid %4; border-radius: 20px; }
        QLabel { color: %5; background: transparent; }
        QLabel#cbTitle { font-size: 26px; font-weight: 700; }
        QLabel#cbSubtitle { font-size: 15px; color: %6; }
        QLabel#cbSection { font-size: 16px; font-weight: 600; }
        QLabel#cbHint { font-size: 13px; color: %6; }
        QLabel#cbStep { font-size: 13px; color: %6; font-weight: 600; }
        QToolButton#cbDiscipline {
            background: %2; color: %5; border: 2px solid %4; border-radius: 14px;
            font-size: 14px; font-weight: 600; padding: 10px 6px 8px 6px;
        }
        QToolButton#cbDiscipline:hover { background: %3; }
        QToolButton#cbDiscipline:checked { background: %8; border: 2px solid %7; }
        QPushButton {
            min-height: 44px; padding: 0 22px; border-radius: 10px; font-size: 15px; font-weight: 600;
            background: %2; color: %5; border: 1px solid %4;
        }
        QPushButton:hover { background: %3; }
        QPushButton#cbPrimary { background: %7; color: %9; border: none; }
        QPushButton#cbPrimary:disabled { background: %4; color: %6; }
        QPushButton#cbDanger { color: %10; }
        QPushButton#cbNav { min-height: 40px; padding: 0 18px; border-radius: 20px; }
        QPushButton#cbNav:checked { background: %8; color: %7; border: 1px solid %7; }
        QRadioButton, QCheckBox { color: %5; font-size: 15px; spacing: 10px; min-height: 34px; }
        QRadioButton::indicator, QCheckBox::indicator { width: 22px; height: 22px; }
        QListWidget { background: %2; color: %5; border: 1px solid %4; border-radius: 10px; font-size: 14px; }
        QListWidget::item { min-height: 40px; padding-left: 6px; }
        QListWidget::item:selected { background: %8; color: %5; }
    )");

    return css.arg(colorName(c.window), colorName(c.surface), colorName(c.surfaceAlt), colorName(c.border),
                   colorName(c.text), colorName(c.textSecondary), colorName(c.accent), colorName(c.accentSoft),
                   colorName(c.accentText))
              .arg(colorName(c.danger));
}
