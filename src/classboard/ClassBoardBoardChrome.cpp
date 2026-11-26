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

#include "ClassBoardBoardChrome.h"

#include "ClassBoardIcons.h"
#include "ClassBoardTheme.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"

#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    const int Margin = 16;      // distance aux bords de la zone du tableau
    const int ButtonSize = 48;  // taille minimale tactile

    QString pillStyleSheet()
    {
        const ClassBoardTheme::Colors c = ClassBoardTheme::colors();

        return QStringLiteral(R"(
            QFrame#%1 { background: %2; border: 1px solid %3; border-radius: 16px; }
            QFrame#%1 QLabel { background: transparent; color: %4; }
            QFrame#%1 QLabel#cbTitle { font-size: 15px; font-weight: 600; }
            QFrame#%1 QLabel#cbSubtitle { font-size: 12px; color: %5; }
            QFrame#%1 QLabel#cbCounter { font-size: 15px; font-weight: 600; min-width: 92px; }
            QFrame#%1 QToolButton {
                background: transparent; border: none; border-radius: 10px; color: %4;
                min-width: 44px; min-height: 44px; padding: 2px 6px; font-size: 14px; font-weight: 600;
            }
            QFrame#%1 QToolButton:hover { background: %6; }
            QFrame#%1 QToolButton:checked, QFrame#%1 QToolButton:pressed { background: %7; color: %8; }
            QFrame#%1 QToolButton:disabled { color: %5; }
            QFrame#%1 QToolButton#cbAccent { background: %7; color: %8; border: 1px solid %8; padding: 2px 14px; }
            QFrame#%1 QToolButton#cbPrimary { background: %9; color: #FFFFFF; padding: 2px 14px; }
            QFrame#%1 QFrame#cbSeparator { background: %3; min-width: 1px; max-width: 1px; margin: 8px 4px; }
        )");
    }

    QFrame* separator(QWidget* parent)
    {
        QFrame* line = new QFrame(parent);
        line->setObjectName("cbSeparator");
        line->setFixedWidth(1);
        return line;
    }
}

ClassBoardBoardChrome::ClassBoardBoardChrome(QWidget* container, QObject* parent)
    : QObject(parent)
    , mContainer(container)
{
    UBMainWindow* w = UBApplication::mainWindow;

    // --- En haut à gauche : menu, nom du document, sauvegarde ------------------
    mDocumentBar = createPill("cbDocumentBar");
    {
        QHBoxLayout* row = new QHBoxLayout(mDocumentBar);
        row->setContentsMargins(6, 6, 12, 6);
        row->setSpacing(6);

        mMenuButton = new QToolButton(mDocumentBar);
        mMenuButton->setIcon(QIcon(":/images/ClassBoard.png"));
        mMenuButton->setIconSize(QSize(36, 36));
        mMenuButton->setToolTip(tr("Menu ClassBoard"));
        mMenuButton->setAccessibleName(tr("Menu ClassBoard"));
        connect(mMenuButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::showMainMenu);
        row->addWidget(mMenuButton);

        QVBoxLayout* texts = new QVBoxLayout();
        texts->setSpacing(0);
        mTitleLabel = new QLabel(mDocumentBar);
        mTitleLabel->setObjectName("cbTitle");
        mSavedLabel = new QLabel(tr("Sauvegarde automatique activée"), mDocumentBar);
        mSavedLabel->setObjectName("cbSubtitle");
        texts->addWidget(mTitleLabel);
        texts->addWidget(mSavedLabel);
        row->addLayout(texts);
    }

    // --- En haut à droite : fonds, documents, web, mode bureau -------------------
    mModeBar = createPill("cbModeBar");
    {
        QHBoxLayout* row = new QHBoxLayout(mModeBar);
        row->setContentsMargins(6, 6, 6, 6);
        row->setSpacing(4);
        row->addWidget(createActionButton(mModeBar, w->actionBackgrounds, true));
        row->addWidget(createActionButton(mModeBar, w->actionDocument, true));
        row->addWidget(createActionButton(mModeBar, w->actionWeb, true));
        row->addWidget(separator(mModeBar));

        QToolButton* desktop = createButton(mModeBar, "desktop", tr("Mode bureau"), true);
        desktop->setObjectName("cbAccent");
        desktop->setToolTip(tr("Écrire par-dessus Windows et les autres logiciels"));
        connect(desktop, &QToolButton::clicked, w->actionDesktop, &QAction::trigger);
        row->addWidget(desktop);
    }

    // --- En bas à gauche : zoom --------------------------------------------------
    mZoomBar = createPill("cbZoomBar");
    {
        QHBoxLayout* row = new QHBoxLayout(mZoomBar);
        row->setContentsMargins(6, 6, 6, 6);
        row->setSpacing(2);

        QToolButton* zoomOut = createButton(mZoomBar, "zoom-out", tr("Zoom arrière"));
        connect(zoomOut, &QToolButton::clicked, this, []() { UBApplication::boardController->zoomOut(); });
        row->addWidget(zoomOut);

        mZoomLabel = new QLabel(mZoomBar);
        mZoomLabel->setObjectName("cbCounter");
        mZoomLabel->setAlignment(Qt::AlignCenter);
        mZoomLabel->setMinimumWidth(64);
        row->addWidget(mZoomLabel);

        QToolButton* zoomIn = createButton(mZoomBar, "zoom-in", tr("Zoom avant"));
        connect(zoomIn, &QToolButton::clicked, this, []() { UBApplication::boardController->zoomIn(); });
        row->addWidget(zoomIn);

        QToolButton* fit = createButton(mZoomBar, "fit", tr("Taille normale"));
        connect(fit, &QToolButton::clicked, this, []() { UBApplication::boardController->zoomRestore(); });
        row->addWidget(fit);
    }

    // --- En bas à droite : pages ---------------------------------------------------
    mPageBar = createPill("cbPageBar");
    {
        QHBoxLayout* row = new QHBoxLayout(mPageBar);
        row->setContentsMargins(6, 6, 6, 6);
        row->setSpacing(2);

        row->addWidget(createActionButton(mPageBar, w->actionBack));
        mPageLabel = new QLabel(mPageBar);
        mPageLabel->setObjectName("cbCounter");
        mPageLabel->setAlignment(Qt::AlignCenter);
        row->addWidget(mPageLabel);
        row->addWidget(createActionButton(mPageBar, w->actionForward));
        row->addWidget(separator(mPageBar));

        QToolButton* newPage = new QToolButton(mPageBar);
        newPage->setIcon(ClassBoardIcons::icon("add", ClassBoardIcons::Style::Toolbar, false, Qt::white));
        newPage->setIconSize(QSize(26, 26));
        newPage->setMinimumHeight(ButtonSize);
        newPage->setText(tr("Page"));
        newPage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        newPage->setFocusPolicy(Qt::NoFocus);
        newPage->setObjectName("cbPrimary");
        newPage->setToolTip(tr("Ajouter une page"));
        connect(newPage, &QToolButton::clicked, w->actionNewPage, &QAction::trigger);
        row->addWidget(newPage);

        QToolButton* duplicate = createButton(mPageBar, "duplicate", tr("Dupliquer la page"));
        connect(duplicate, &QToolButton::clicked, w->actionDuplicatePage, &QAction::trigger);
        row->addWidget(duplicate);
    }

    // --- Au-dessus de la barre des outils : couleurs et épaisseurs ------------------
    mInkBar = createPill("cbInkBar");
    {
        QHBoxLayout* row = new QHBoxLayout(mInkBar);
        row->setContentsMargins(10, 4, 10, 4);
        row->setSpacing(8);

        // Les groupes d'OpenBoard exigent une barre d'outils comme parent initial :
        // ils sont ensuite déplacés dans la barre ClassBoard (même principe que le mode bureau).
        mColorChoice = new UBToolbarButtonGroup(w->boardToolBar,
                                                {w->actionColor0, w->actionColor1, w->actionColor2, w->actionColor3, w->actionColor4},
                                                "classboard");
        mColorChoice->displayText(false);
        connect(mColorChoice, SIGNAL(activated(int)), UBApplication::boardController, SLOT(setColorIndex(int)));
        connect(UBDrawingController::drawingController(), SIGNAL(colorIndexChanged(int)), mColorChoice, SLOT(setCurrentIndex(int)));
        connect(UBDrawingController::drawingController(), SIGNAL(colorPaletteChanged()), mColorChoice, SLOT(colorPaletteChanged()));
        mColorChoice->colorPaletteChanged();
        mColorChoice->setCurrentIndex(UBSettings::settings()->penColorIndex());
        row->addWidget(mColorChoice);

        mLineWidthChoice = new UBToolbarButtonGroup(w->boardToolBar,
                                                    {w->actionLineSmall, w->actionLineMedium, w->actionLineLarge},
                                                    "classboard");
        mLineWidthChoice->displayText(false);
        connect(mLineWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setLineWidthIndex(int)));
        connect(UBDrawingController::drawingController(), SIGNAL(lineWidthIndexChanged(int)), mLineWidthChoice, SLOT(setCurrentIndex(int)));
        mLineWidthChoice->setCurrentIndex(UBSettings::settings()->penWidthIndex());
        row->addWidget(mLineWidthChoice);

        mEraserWidthChoice = new UBToolbarButtonGroup(w->boardToolBar,
                                                      {w->actionEraserSmall, w->actionEraserMedium, w->actionEraserLarge},
                                                      "classboard");
        mEraserWidthChoice->displayText(false);
        connect(mEraserWidthChoice, SIGNAL(activated(int)), UBDrawingController::drawingController(), SLOT(setEraserWidthIndex(int)));
        mEraserWidthChoice->setCurrentIndex(UBSettings::settings()->eraserWidthIndex());
        row->addWidget(mEraserWidthChoice);

        row->addWidget(separator(mInkBar));
        QToolButton* clearPage = createButton(mInkBar, "clear-page", tr("Effacer la page"));
        connect(clearPage, &QToolButton::clicked, w->actionClearPage, &QAction::trigger);
        row->addWidget(clearPage);
    }

    // --- Raccourcis clavier : la barre d'OpenBoard étant masquée, ses actions
    //     sont rattachées à la fenêtre pour que Ctrl+Z, flèches, etc. restent actifs.
    w->addActions(w->boardToolBar->actions());

    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &ClassBoardBoardChrome::refreshDocumentInfo);
    connect(UBApplication::boardController, &UBBoardController::zoomChanged, this, &ClassBoardBoardChrome::refreshZoom);
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this, &ClassBoardBoardChrome::stylusToolChanged);

    // Nombre de pages et nom du document : rafraîchis régulièrement (ajouts,
    // suppressions et renommages n'émettent pas tous un signal dédié).
    QTimer* refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &ClassBoardBoardChrome::refreshDocumentInfo);
    refresh->start(1000);

    refreshDocumentInfo();
    refreshZoom();
    stylusToolChanged(UBDrawingController::drawingController()->stylusTool());
}

QFrame* ClassBoardBoardChrome::createPill(const QString& name)
{
    QFrame* pill = new QFrame(mContainer);
    pill->setObjectName(name);
    pill->setStyleSheet(pillStyleSheet().arg(name,
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().surface),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().border),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().text),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().textSecondary),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().surfaceAlt),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().accentSoft),
                                            ClassBoardTheme::colorName(ClassBoardTheme::colors().accent),
                                            // bouton principal : bleu assez foncé pour un texte blanc lisible
                                            ClassBoardTheme::isDark() ? QStringLiteral("#2B6CB0") : ClassBoardTheme::colorName(ClassBoardTheme::colors().accent)));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(pill);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, ClassBoardTheme::isDark() ? 110 : 40));
    pill->setGraphicsEffect(shadow);

    return pill;
}

QToolButton* ClassBoardBoardChrome::createActionButton(QWidget* parent, QAction* action, bool showText)
{
    QToolButton* button = new QToolButton(parent);
    button->setDefaultAction(action);
    button->setIconSize(QSize(26, 26));
    button->setMinimumHeight(ButtonSize);
    button->setToolButtonStyle(showText ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}

QToolButton* ClassBoardBoardChrome::createButton(QWidget* parent, const QString& iconName, const QString& text, bool showText)
{
    QToolButton* button = new QToolButton(parent);
    button->setIcon(ClassBoardIcons::icon(iconName, ClassBoardIcons::Style::Toolbar));
    button->setIconSize(QSize(26, 26));
    button->setMinimumHeight(ButtonSize);
    button->setText(text);
    button->setToolTip(text);
    button->setAccessibleName(text);
    button->setToolButtonStyle(showText ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
    button->setFocusPolicy(Qt::NoFocus);
    return button;
}

void ClassBoardBoardChrome::setInkAnchor(QWidget* anchor)
{
    if (mInkAnchor)
        mInkAnchor->removeEventFilter(this);

    mInkAnchor = anchor;

    if (mInkAnchor)
        mInkAnchor->installEventFilter(this);

    placeInkBar();
}

bool ClassBoardBoardChrome::eventFilter(QObject* watched, QEvent* event)
{
    // La barre des outils peut être déplacée à la main : les autres barres s'adaptent.
    if (watched == mInkAnchor && (event->type() == QEvent::Move || event->type() == QEvent::Resize) && mArea.isValid())
        place(mArea);

    return QObject::eventFilter(watched, event);
}

void ClassBoardBoardChrome::placeInkBar()
{
    if (!mInkAnchor)
        return;

    mInkBar->adjustSize();
    const QRect anchor = mInkAnchor->geometry();
    int y = anchor.top() - mInkBar->height() - 10;

    // Barre des outils en haut de l'écran : couleurs en dessous
    if (y < mArea.top() + Margin)
        y = anchor.bottom() + 10;

    mInkBar->move(anchor.center().x() - mInkBar->width() / 2, y);
    mInkBar->raise();
}

void ClassBoardBoardChrome::place(const QRect& area)
{
    mArea = area;

    for (QFrame* pill : {mDocumentBar, mModeBar, mZoomBar, mPageBar, mInkBar})
        pill->adjustSize();

    mDocumentBar->move(area.left() + Margin, area.top() + Margin);
    mModeBar->move(area.right() - Margin - mModeBar->width(), area.top() + Margin);
    mZoomBar->move(area.left() + Margin, area.bottom() - Margin - mZoomBar->height());
    mPageBar->move(area.right() - Margin - mPageBar->width(), area.bottom() - Margin - mPageBar->height());

    placeInkBar();

    // Écran étroit : les barres de coin remontent au-dessus de la barre des outils
    // (et de la barre des couleurs) plutôt que de les recouvrir.
    for (QFrame* pill : {mZoomBar, mPageBar})
    {
        for (QWidget* obstacle : {static_cast<QWidget*>(mInkAnchor), static_cast<QWidget*>(mInkBar)})
        {
            if (!obstacle || !obstacle->isVisible())
                continue;

            const QRect busy = obstacle->geometry().adjusted(-8, -8, 8, 8);

            if (pill->geometry().intersects(busy))
                pill->move(pill->x(), busy.top() - pill->height());
        }
    }

    raiseAll();
}

QRect ClassBoardBoardChrome::leftFreeArea() const
{
    const int top = mArea.top() + TopReservedHeight;
    const int bottom = mZoomBar->isVisible() ? mZoomBar->y() - 8 : mArea.bottom();
    return QRect(mArea.left(), top, mArea.width(), qMax(0, bottom - top));
}

void ClassBoardBoardChrome::setVisible(bool visible)
{
    mVisible = visible;

    for (QFrame* pill : {mDocumentBar, mModeBar, mZoomBar, mPageBar})
        pill->setVisible(visible);

    stylusToolChanged(UBDrawingController::drawingController()->stylusTool());
}

void ClassBoardBoardChrome::raiseAll()
{
    for (QFrame* pill : {mDocumentBar, mModeBar, mZoomBar, mPageBar, mInkBar})
        pill->raise();
}

void ClassBoardBoardChrome::refreshDocumentInfo()
{
    UBBoardController* board = UBApplication::boardController;

    if (!board || !board->selectedDocument())
        return;

    const QString name = board->selectedDocument()->name();
    if (mTitleLabel->text() != name)
    {
        mTitleLabel->setText(name.isEmpty() ? tr("Document sans titre") : name);
        mDocumentBar->adjustSize();
    }

    const int count = qMax(1, board->selectedDocument()->pageCount());
    const QString pages = tr("Page %1 / %2").arg(board->activeSceneIndex() + 1).arg(count);
    if (mPageLabel->text() != pages)
    {
        mPageLabel->setText(pages);
        if (mArea.isValid())
            place(mArea);
    }
}

void ClassBoardBoardChrome::markSaved()
{
    mSavedLabel->setText(tr("Enregistré automatiquement · %1").arg(QTime::currentTime().toString("HH:mm")));
    mDocumentBar->adjustSize();
}

void ClassBoardBoardChrome::refreshZoom()
{
    const qreal zoom = UBApplication::boardController ? UBApplication::boardController->currentZoom() : 1.0;
    mZoomLabel->setText(QString("%1 %").arg(qRound(zoom * 100)));
}

void ClassBoardBoardChrome::stylusToolChanged(int tool)
{
    const bool drawing = tool == UBStylusTool::Pen || tool == UBStylusTool::Marker || tool == UBStylusTool::Line;
    const bool erasing = tool == UBStylusTool::Eraser;

    mColorChoice->setVisible(drawing);
    mLineWidthChoice->setVisible(drawing);
    mEraserWidthChoice->setVisible(erasing);
    mInkBar->setVisible(mVisible && (drawing || erasing));

    placeInkBar();
}

void ClassBoardBoardChrome::showMainMenu()
{
    // Le menu ClassBoard est construit par UBApplication::decorateActionMenu sur le
    // bouton « ClassBoard » de la barre d'OpenBoard (masquée) : on le réutilise.
    const auto objects = UBApplication::mainWindow->actionMenu->associatedObjects();

    for (QObject* object : objects)
    {
        QToolButton* button = qobject_cast<QToolButton*>(object);

        if (button && button->menu())
        {
            button->menu()->popup(mMenuButton->mapToGlobal(QPoint(0, mMenuButton->height() + 6)));
            return;
        }
    }
}
