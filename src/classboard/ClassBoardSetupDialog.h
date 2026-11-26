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

#include <QDialog>
#include <QMap>

class QButtonGroup;
class QCheckBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class QToolButton;

/**
 * @brief Accueil de premier lancement (1 écran rapide) et réglages « Mes matières ».
 *
 * Mode FirstRun : un seul écran chaleureux pour choisir sa ou ses matières
 * (Anglais, Français, etc.) et commencer immédiatement.
 * Mode Settings : accès direct aux matières et à l'apparence (côté de la barre,
 * thème clair/ardoise, taille des boutons, libellés), appliqués en direct.
 */
class ClassBoardSetupDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { FirstRun, Settings };

    explicit ClassBoardSetupDialog(Mode mode, QWidget* parent = nullptr);

    /// Affiche l'accueil si le premier lancement n'a pas encore eu lieu.
    /// Retourne vrai si l'écran a été affiché.
    static bool runFirstLaunchIfNeeded();

private slots:
    void skip();
    void saveAndClose();
    void resetProfile();
    void updateState();

private:
    QWidget* buildHeader();
    QWidget* buildDisciplinesPage();
    QWidget* buildAppearancePage();
    QWidget* buildFooter();

    QStringList selectedDisciplines() const;
    void loadFromProfile();
    void save();

    Mode mMode;

    QStackedWidget* mPages = nullptr;
    QButtonGroup* mNavGroup = nullptr;

    QMap<QString, QToolButton*> mDisciplineButtons;
    QLabel* mSelectionSummary = nullptr;

    QRadioButton* mSideRight = nullptr;
    QRadioButton* mSideLeft = nullptr;
    QRadioButton* mThemeLight = nullptr;
    QRadioButton* mThemeDark = nullptr;
    QRadioButton* mThemeSystem = nullptr;
    QRadioButton* mSize100 = nullptr;
    QRadioButton* mSize125 = nullptr;
    QRadioButton* mSize150 = nullptr;
    QCheckBox* mShowLabels = nullptr;

    QPushButton* mPrimaryButton = nullptr;
};
