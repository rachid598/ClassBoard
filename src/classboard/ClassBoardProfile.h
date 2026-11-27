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

#include <QString>
#include <QStringList>
#include <QVariant>

/**
 * @brief Profil enseignant de ClassBoard (disciplines, thème, favoris...).
 *
 * Le profil est stocké dans un fichier INI propre à l'utilisateur Windows :
 *   - %APPDATA%\ClassBoard\ClassBoardProfile.ini (profil itinérant : il suit
 *     l'enseignant d'un poste à l'autre sur un réseau Kwartz) ;
 *   - en mode portable (fichier "portable.txt" à côté de l'exécutable) :
 *     <dossier de l'application>\settings\ClassBoardProfile.ini.
 *
 * Toutes les méthodes sont statiques et ne dépendent que de QtCore, ce qui
 * permet de les tester unitairement sans lancer l'application.
 */
class ClassBoardProfile
{
public:
    enum class Theme { Light, Dark, System };
    enum class ToolbarSide { Right, Left };

    // --- Emplacements -------------------------------------------------------
    static bool isPortable();
    static QString portableDataDirectory();
    static QString profileDirectory();
    static QString profileFile();

    /// Pour les tests : force l'emplacement du fichier de profil (vide = défaut).
    static void setProfileFileOverride(const QString& path);

    // --- Premier lancement -------------------------------------------------
    static bool isFirstRunCompleted();
    static void setFirstRunCompleted(bool completed);

    // --- Disciplines -------------------------------------------------------
    static QStringList disciplines();
    static void setDisciplines(const QStringList& ids);

    // --- Apparence ---------------------------------------------------------
    static Theme theme();
    static void setTheme(Theme theme);
    static QString themeToString(Theme theme);
    static Theme themeFromString(const QString& value);

    /// Côté de la barre verticale du tableau (droite par défaut, gauche pour gauchers).
    static ToolbarSide toolbarSide();
    static void setToolbarSide(ToolbarSide side);
    static QString toolbarSideToString(ToolbarSide side);
    static ToolbarSide toolbarSideFromString(const QString& value);

    /// Taille des boutons de la barre d'outils, en pourcentage (100, 125 ou 150).
    static int toolbarScale();
    static void setToolbarScale(int percent);

    static bool showToolLabels();
    static void setShowToolLabels(bool show);

    // --- Langue ------------------------------------------------------------
    /// Code de langue de l'interface (ex. "fr", "en"). Vide = français par défaut.
    static QString interfaceLanguage();
    static void setInterfaceLanguage(const QString& language);

    // --- Stockage (lecteur réseau Kwartz, clé USB...) ----------------------
    /// Dossier personnalisé pour les cours et documents (vide = dossier local par défaut).
    static QString documentsDirectory();
    static void setDocumentsDirectory(const QString& path);

    // --- Favoris -----------------------------------------------------------
    /// Favoris personnalisés. Vide = favoris proposés par les disciplines.
    static QStringList customFavorites();
    static void setCustomFavorites(const QStringList& favorites);
    /// Favoris effectivement affichés (personnalisés ou déduits des disciplines).
    static QStringList effectiveFavorites();

    static bool showFavoritesBar();
    static void setShowFavoritesBar(bool show);

    // --- Réinitialisation --------------------------------------------------
    static void reset();

private:
    static QVariant value(const QString& key, const QVariant& defaultValue = QVariant());
    static void setValue(const QString& key, const QVariant& value);
};
