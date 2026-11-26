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

#pragma once

#include <QColor>
#include <QIcon>
#include <QSize>
#include <QString>

/**
 * @brief Thème visuel de ClassBoard (clair / sombre), inspiré de Fluent Design.
 *
 * Centralise les couleurs utilisées par les nouveaux composants ClassBoard et
 * applique la feuille de style globale (etc/ClassBoard.css, complétée par
 * etc/ClassBoard-dark.css en mode sombre).
 */
class ClassBoardTheme
{
public:
    struct Colors
    {
        QColor window;       // fond de l'application
        QColor surface;      // palettes, cartes, dialogues
        QColor surfaceAlt;   // survol, zones secondaires
        QColor border;
        QColor text;
        QColor textSecondary;
        QColor accent;
        QColor accentText;   // texte sur fond accent
        QColor accentSoft;   // fond d'un bouton actif
        QColor danger;
    };

    static bool isDark();
    static Colors colors();

    /// Applique le thème courant à l'application (style, palette Qt, feuille de style).
    static void apply();

    /// Taille d'icône des barres d'outils selon le réglage "taille des boutons".
    static QSize scaledIconSize(int baseSize);

    /// Feuille de style des dialogues ClassBoard (assistant, paramètres).
    static QString dialogStyleSheet();

    /// Icône SVG recolorée selon le thème (le SVG doit utiliser "currentColor").
    static QIcon tintedIcon(const QString& svgResource, const QColor& color, int size = 48);

    static QString colorName(const QColor& color);
};
