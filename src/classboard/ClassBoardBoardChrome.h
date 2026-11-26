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

#include <QObject>
#include <QRect>

class QAction;
class QFrame;
class QLabel;
class QToolButton;
class QWidget;
class UBToolbarButtonGroup;

/**
 * @brief Habillage du tableau en mode « plein écran » (maquette ClassBoard).
 *
 * Remplace la grande barre d'outils d'OpenBoard par des barres flottantes
 * discrètes posées aux coins de la page, pour laisser la place à l'écriture :
 *
 *   - en haut à gauche : menu ClassBoard, nom du document, sauvegarde ;
 *   - en haut à droite : fonds, documents, web, mode bureau ;
 *   - en bas à gauche  : zoom ;
 *   - en bas à droite  : navigation entre les pages, nouvelle page ;
 *   - au-dessus de la barre des outils : couleurs et épaisseurs du stylo.
 *
 * Toutes les commandes réutilisent les actions existantes d'OpenBoard
 * (aucune logique de tableau n'est dupliquée).
 */
class ClassBoardBoardChrome : public QObject
{
    Q_OBJECT

public:
    /// Hauteur réservée en haut du tableau par les barres flottantes.
    static constexpr int TopReservedHeight = 96;

    ClassBoardBoardChrome(QWidget* container, QObject* parent = nullptr);

    /// Place les barres dans la zone utile du tableau.
    void place(const QRect& area);

    /// Barre des outils au-dessus de laquelle s'affichent couleurs et épaisseurs.
    void setInkAnchor(QWidget* anchor);

    void setVisible(bool visible);

public slots:
    void refreshDocumentInfo();
    void markSaved();

private slots:
    void refreshZoom();
    void stylusToolChanged(int tool);
    void showMainMenu();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void placeInkBar();
    QFrame* createPill(const QString& name);
    QToolButton* createActionButton(QWidget* parent, QAction* action, bool showText = false);
    QToolButton* createButton(QWidget* parent, const QString& iconName, const QString& text, bool showText = false);
    void raiseAll();

    QWidget* mContainer;

    QFrame* mDocumentBar = nullptr;
    QFrame* mModeBar = nullptr;
    QFrame* mZoomBar = nullptr;
    QFrame* mPageBar = nullptr;
    QFrame* mInkBar = nullptr;

    QToolButton* mMenuButton = nullptr;
    QLabel* mTitleLabel = nullptr;
    QLabel* mSavedLabel = nullptr;
    QLabel* mZoomLabel = nullptr;
    QLabel* mPageLabel = nullptr;

    UBToolbarButtonGroup* mColorChoice = nullptr;
    UBToolbarButtonGroup* mLineWidthChoice = nullptr;
    UBToolbarButtonGroup* mEraserWidthChoice = nullptr;

    QRect mArea;
    QWidget* mInkAnchor = nullptr;
    bool mVisible = true;
};
