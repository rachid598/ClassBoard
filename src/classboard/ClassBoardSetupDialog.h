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

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;
class QRadioButton;
class QStackedWidget;
class QToolButton;

/**
 * @brief Assistant de premier lancement et écran "Mon profil ClassBoard".
 *
 * Mode FirstRun : trois étapes (disciplines, apparence, favoris) affichées au
 * tout premier démarrage. Mode Settings : mêmes pages, accessibles à tout
 * moment depuis le menu, avec un bouton de réinitialisation.
 */
class ClassBoardSetupDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode { FirstRun, Settings };

    explicit ClassBoardSetupDialog(Mode mode, QWidget* parent = nullptr);

    /// Affiche l'assistant si le premier lancement n'a pas encore eu lieu.
    /// Retourne vrai si l'assistant a été affiché.
    static bool runFirstLaunchIfNeeded();

    /// Vrai si l'utilisateur a modifié un réglage qui nécessite un redémarrage.
    bool restartRequired() const { return mRestartRequired; }

private slots:
    void goNext();
    void goBack();
    void skip();
    void saveAndClose();
    void resetProfile();
    void updateState();
    void restoreDisciplineFavorites();

private:
    QWidget* buildHeader();
    QWidget* buildDisciplinesPage();
    QWidget* buildAppearancePage();
    QWidget* buildFavoritesPage();
    QWidget* buildFooter();

    QStringList selectedDisciplines() const;
    void loadFromProfile();
    void fillFavorites(const QStringList& favorites);
    void refreshFavoritesIfUntouched();
    QStringList checkedFavorites() const;
    void save();

    Mode mMode;
    bool mRestartRequired = false;

    QStackedWidget* mPages = nullptr;
    QLabel* mStepLabel = nullptr;
    QList<QLabel*> mStepBars;
    QButtonGroup* mNavGroup = nullptr;

    QMap<QString, QToolButton*> mDisciplineButtons;
    QLabel* mSelectionSummary = nullptr;

    QRadioButton* mThemeLight = nullptr;
    QRadioButton* mThemeDark = nullptr;
    QRadioButton* mThemeSystem = nullptr;
    QRadioButton* mSize100 = nullptr;
    QRadioButton* mSize125 = nullptr;
    QRadioButton* mSize150 = nullptr;
    QCheckBox* mShowLabels = nullptr;
    QCheckBox* mShowFavoritesBar = nullptr;

    QListWidget* mFavorites = nullptr;
    bool mFavoritesTouched = false;   // l'enseignant a modifié la liste des favoris
    bool mFillingFavorites = false;

    QPushButton* mSkipButton = nullptr;
    QPushButton* mBackButton = nullptr;
    QPushButton* mNextButton = nullptr;
    QPushButton* mResetButton = nullptr;
};
