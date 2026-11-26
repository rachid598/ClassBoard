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

#include <QFrame>
#include <QList>

class QToolButton;

/**
 * @brief Sélecteur moderne des fonds de page (Blanc, Quadrillé, Seyès, Ardoise...).
 *
 * Remplace l'ancienne palette d'OpenBoard à vignettes cornées par des cartes
 * plates épurées dessinées vectoriellement.
 */
class ClassBoardBackgroundPopover : public QFrame
{
    Q_OBJECT

public:
    explicit ClassBoardBackgroundPopover(QWidget* parent = nullptr);

    void applyTheme();
    void syncSelection();

signals:
    void backgroundChosen();

private:
    struct Option
    {
        bool dark = false;
        int kind = 0; // UBPageBackground
        QToolButton* button = nullptr;
    };

    QIcon buildPreviewIcon(bool dark, int kind) const;
    void choose(bool dark, int kind);

    QList<Option> mOptions;
};
