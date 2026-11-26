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

#include <QIcon>
#include <QString>

class UBMainWindow;

/**
 * @brief Jeu d'icônes ClassBoard (trait fin, style Fluent), clair ou sombre.
 *
 * Les icônes sont dessinées à partir des SVG de resources/classboard/tools
 * et remplacent à l'exécution celles d'OpenBoard : les fichiers d'origine
 * restent intacts, ce qui facilite la reprise des évolutions d'OpenBoard.
 */
class ClassBoardIcons
{
public:
    enum class Style
    {
        Tool,       // palette des stylets : fond coloré quand l'outil est actif
        Toolbar,    // barre supérieure : glyphe seul
        Favorite    // barre de favoris : glyphe dans une pastille
    };

    /// Icône "nom" (fichier :/classboard/tools/<nom>.svg) dans le style demandé.
    static QIcon icon(const QString& name, Style style = Style::Tool, bool withMenuArrow = false,
                      const QColor& color = QColor());

    /// Icône bicolore moderne dans une pastille pastel (état normal + actif).
    static QIcon duotoneIcon(const QString& name, const QColor& fgColor, const QColor& bgColor,
                             const QColor& activeFg = QColor(), const QColor& activeBg = QColor());

    /// Nom d'icône associé à un favori (ex. "openboardtool://ruler" -> "ruler"). Vide si inconnu.
    static QString iconNameForFavorite(const QString& favorite);

    /// Remplace les icônes des actions de la fenêtre principale.
    static void applyToMainWindow(UBMainWindow* mainWindow);

    /// Icônes stylo / surligneur / gomme, avec ou sans flèche de sous-menu (mode bureau).
    static void applyDrawingToolIcons(UBMainWindow* mainWindow, bool withMenuArrow);
};
