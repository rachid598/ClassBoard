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
#include <QObject>
#include <QRect>

class ClassBoardBackgroundPopover;
class ClassBoardToolsDrawer;
class QAction;
class QFrame;
class QLabel;
class QPushButton;
class QToolButton;
class QWidget;

/**
 * @brief Interface flottante moderne du tableau ClassBoard.
 *
 * Remplace les barres traditionnelles par un habillage léger et aéré :
 *   - en haut à gauche  : logo ClassBoard, titre du cours, témoin de sauvegarde ;
 *   - en haut à droite  : Fonds, Documents, Bureau, bascule Clair/Ardoise, menu ;
 *   - sur le côté droit (ou gauche) : barre verticale de 6 outils essentiels
 *     (Choisir, Écrire, Surligner, Gommer, Texte, Outils) + Annuler/Rétablir ;
 *   - bulle contextuelle à côté de l'outil actif : couleurs rondes, épaisseurs,
 *     ou tailles de gomme + Tout effacer ;
 *   - en bas à droite   : navigation compacte des pages (< 1 / 1 > + Page).
 *
 * Tout le bas du tableau reste entièrement dégagé pour écrire.
 */
class ClassBoardBoardChrome : public QObject
{
    Q_OBJECT

public:
    /// Hauteur réservée en haut du tableau par les barres flottantes.
    static constexpr int TopReservedHeight = 84;

    ClassBoardBoardChrome(QWidget* container, QObject* parent = nullptr);

    /// Place les barres dans la zone utile du tableau.
    void place(const QRect& area);

    /// Conservé pour compatibilité d'appel (la barre verticale est intégrée).
    void setInkAnchor(QWidget* anchor);

    void setVisible(bool visible);

    /// Zone libre sur le côté opposé à la barre d'outils.
    QRect leftFreeArea() const;

    /// Applique immédiatement les changements de profil (thème, côté, libellés, disciplines).
    void reloadFromProfile();

public slots:
    void refreshDocumentInfo();
    void markSaved();

private slots:
    void refreshZoom();
    void stylusToolChanged(int tool);
    void refreshColorSwatches();
    void refreshWidthSelection();
    void toggleBackgroundPopover();
    void toggleToolsDrawer();
    void toggleDarkMode();
    void showMainMenu();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void applyStyles();
    void rebuildToolIcons();
    void placeToolPopover();
    void placeBackgroundPopover();
    void placeToolsDrawer();
    void closePopovers();
    void selectColor(int index);
    void selectWidth(int index);
    void selectTool(int tool);
    QFrame* createPill(const QString& name, int radius = 20);
    QToolButton* createToolRailButton(const QString& label, const QString& tooltip);
    QToolButton* createIconButton(QWidget* parent, const QString& iconName, const QString& tooltip);
    void raiseAll();

    QWidget* mContainer = nullptr;

    QFrame* mDocumentBar = nullptr;
    QFrame* mModeBar = nullptr;
    QFrame* mToolRail = nullptr;
    QFrame* mToolPopover = nullptr;
    QFrame* mZoomBar = nullptr;
    QFrame* mPageBar = nullptr;

    ClassBoardBackgroundPopover* mBackgroundPopover = nullptr;
    ClassBoardToolsDrawer* mToolsDrawer = nullptr;

    QToolButton* mMenuButton = nullptr;
    QLabel* mTitleLabel = nullptr;
    QLabel* mSaveDot = nullptr;

    QToolButton* mBackgroundsButton = nullptr;
    QToolButton* mDocumentsButton = nullptr;
    QToolButton* mDesktopButton = nullptr;
    QToolButton* mThemeButton = nullptr;
    QToolButton* mMoreButton = nullptr;

    // 6 boutons essentiels de la barre verticale
    QToolButton* mSelectBtn = nullptr;
    QToolButton* mPenBtn = nullptr;
    QToolButton* mMarkerBtn = nullptr;
    QToolButton* mEraserBtn = nullptr;
    QToolButton* mTextBtn = nullptr;
    QToolButton* mToolsBtn = nullptr;
    QToolButton* mUndoBtn = nullptr;
    QToolButton* mRedoBtn = nullptr;

    // Contenu de la bulle contextuelle (couleurs / épaisseurs / tout effacer)
    QWidget* mColorsRow = nullptr;
    QList<QToolButton*> mColorButtons;
    QWidget* mWidthsRow = nullptr;
    QList<QToolButton*> mWidthButtons;
    QPushButton* mClearPageBtn = nullptr;

    QLabel* mZoomLabel = nullptr;
    QToolButton* mPageLabelBtn = nullptr;
    QToolButton* mNewPageBtn = nullptr;
    QToolButton* mPrevPageBtn = nullptr;
    QToolButton* mNextPageBtn = nullptr;

    QRect mArea;
    bool mVisible = true;
    bool mPopoverSuppressed = false;
    int mCurrentTool = -1;
};
