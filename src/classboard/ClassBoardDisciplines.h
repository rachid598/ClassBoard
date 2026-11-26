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

#include <QList>
#include <QString>
#include <QStringList>

/**
 * @brief Une discipline enseignée et les outils qu'elle met en avant.
 *
 * Les favoris sont des identifiants d'outils :
 *   - "openboardtool://ruler"... : instruments natifs du moteur OpenBoard ;
 *   - "widget:Minuteur"          : application web de la bibliothèque
 *                                  (library/applications/Minuteur.wgt).
 *
 * Une discipline ne cloisonne rien : elle ne fait que choisir ce qui est
 * proposé en premier. Tous les outils restent accessibles.
 */
struct ClassBoardDiscipline
{
    QString id;
    const char* name = nullptr;     // nom source, traduit via displayName()
    QString icon;                   // ressource Qt (":/classboard/disciplines/...")
    QStringList favorites;
    bool suggestsSeyes = false;     // fond Seyès proposé par défaut

    QString displayName() const;
    bool isValid() const { return !id.isEmpty(); }
};

class ClassBoardDisciplines
{
public:
    static const QList<ClassBoardDiscipline>& all();
    static ClassBoardDiscipline byId(const QString& id);

    /// Filtre une liste d'identifiants : conserve les identifiants connus, sans doublon, dans l'ordre.
    static QStringList validIds(const QStringList& ids);

    /// Favoris proposés pour un ensemble de disciplines (fusion équitable, sans doublon).
    static QStringList favoritesFor(const QStringList& disciplineIds, int maximum = 9);

    /// Vrai si au moins une discipline suggère le fond Seyès.
    static bool suggestsSeyes(const QStringList& disciplineIds);

    /// Nom lisible d'un favori ("openboardtool://ruler" -> "Règle").
    static QString favoriteDisplayName(const QString& favorite);

    static const QString WidgetPrefix;
};
