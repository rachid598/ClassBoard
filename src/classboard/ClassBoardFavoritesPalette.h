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

#include "gui/UBActionPalette.h"

/**
 * @brief Barre verticale des outils favoris, à gauche du tableau.
 *
 * Son contenu dépend des disciplines de l'enseignant (ou de ses favoris
 * personnalisés). Un clic pose l'outil au centre de la page courante.
 */
class ClassBoardFavoritesPalette : public UBActionPalette
{
    Q_OBJECT

public:
    explicit ClassBoardFavoritesPalette(QWidget* parent = nullptr);

    /// Reconstruit la barre à partir du profil.
    void reload();

    /// Positionne la barre contre le bord gauche, centrée verticalement.
    void placeInContainer(const QRect& area);

signals:
    void editRequested();

private slots:
    void favoriteTriggered();

private:
    static QIcon iconForFavorite(const QString& favorite, QString* label, QUrl* url);
};
