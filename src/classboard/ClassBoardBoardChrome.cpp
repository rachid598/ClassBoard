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

#include "ClassBoardBackgroundPopover.h"
#include "ClassBoardIcons.h"
#include "ClassBoardProfile.h"
#include "ClassBoardTheme.h"
#include "ClassBoardToolsDrawer.h"

#include "adaptors/UBExportPDF.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "board/UBDrawingController.h"
#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"
#include "document/UBDocumentProxy.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBLeftPalette.h"
#include "gui/UBMainWindow.h"

#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTime>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    const int Margin = 18;
    const int SwatchCount = 5;

    QIcon swatchIcon(const QColor& fill, bool selected, bool darkUi)
    {
        QIcon icon;
        for (int scale : {1, 2})
        {
            const int px = 32 * scale;
            QPixmap pm(px, px);
            pm.fill(Qt::transparent);

            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing);

            const qreal center = px / 2.0;
            if (selected)
            {
                p.setPen(QPen(darkUi ? QColor("#60A5FA") : QColor("#4F46E5"), 2.2 * scale));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(QPointF(center, center), 14.0 * scale, 14.0 * scale);
            }

            const qreal r = (selected ? 10.5 : 11.5) * scale;
            QColor opaque = fill;
            opaque.setAlpha(255);
            p.setBrush(opaque);
            const bool veryLight = opaque.lightness() > 225;
            p.setPen(veryLight ? QPen(QColor("#CBD5E1"), 1.2 * scale) : Qt::NoPen);
            p.drawEllipse(QPointF(center, center), r, r);
            p.end();

            pm.setDevicePixelRatio(scale);
            icon.addPixmap(pm);
        }
        return icon;
    }

    QIcon dotWidthIcon(int level, bool selected, bool erasing, bool darkUi)
    {
        static const qreal radii[3] = {3.2, 5.4, 8.2};
        const qreal r = radii[qBound(0, level, 2)];

        QIcon icon;
        for (int scale : {1, 2})
        {
            const int px = 30 * scale;
            QPixmap pm(px, px);
            pm.fill(Qt::transparent);

            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing);

            const qreal center = px / 2.0;
            if (selected)
            {
                p.setPen(Qt::NoPen);
                p.setBrush(darkUi ? QColor(96, 165, 250, 48) : QColor("#EEF2FF"));
                p.drawEllipse(QPointF(center, center), 13.5 * scale, 13.5 * scale);
            }

            const QColor ink = selected
                ? (darkUi ? QColor("#60A5FA") : QColor("#4F46E5"))
                : (darkUi ? QColor("#CBD5E1") : QColor("#475569"));

            if (erasing)
            {
                p.setPen(QPen(ink, 1.8 * scale));
                p.setBrush(selected ? QColor(ink.red(), ink.green(), ink.blue(), 45) : Qt::NoBrush);
            }
            else
            {
                p.setPen(Qt::NoPen);
                p.setBrush(ink);
            }

            p.drawEllipse(QPointF(center, center), r * scale, r * scale);
            p.end();

            pm.setDevicePixelRatio(scale);
            icon.addPixmap(pm);
        }
        return icon;
    }

    QFrame* hairlineH(QWidget* parent)
    {
        QFrame* line = new QFrame(parent);
        line->setObjectName("cbDividerH");
        line->setFixedHeight(1);
        return line;
    }

    QFrame* hairlineV(QWidget* parent)
    {
        QFrame* line = new QFrame(parent);
        line->setObjectName("cbDividerV");
        line->setFixedWidth(1);
        return line;
    }
}

ClassBoardBoardChrome::ClassBoardBoardChrome(QWidget* container, QObject* parent)
    : QObject(parent)
    , mContainer(container)
{
    UBMainWindow* w = UBApplication::mainWindow;

    // --- En haut à gauche : logo + nom du cours + témoin discret de sauvegarde ---
    mDocumentBar = createPill("cbDocumentBar", 22);
    {
        QHBoxLayout* row = new QHBoxLayout(mDocumentBar);
        row->setContentsMargins(8, 6, 16, 6);
        row->setSpacing(10);

        mMenuButton = new QToolButton(mDocumentBar);
        mMenuButton->setObjectName("cbLogoBtn");
        mMenuButton->setIcon(QIcon(":/images/ClassBoard.png"));
        mMenuButton->setIconSize(QSize(34, 34));
        mMenuButton->setFixedSize(40, 40);
        mMenuButton->setCursor(Qt::PointingHandCursor);
        mMenuButton->setFocusPolicy(Qt::NoFocus);
        mMenuButton->setToolTip(tr("Menu ClassBoard"));
        connect(mMenuButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::showMainMenu);
        row->addWidget(mMenuButton);

        mTitleButton = new QToolButton(mDocumentBar);
        mTitleButton->setObjectName("cbTitleBtn");
        mTitleButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        mTitleButton->setCursor(Qt::PointingHandCursor);
        mTitleButton->setFocusPolicy(Qt::NoFocus);
        mTitleButton->setToolTip(tr("Cliquer pour renommer ce cours"));
        connect(mTitleButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::renameCurrentDocument);
        row->addWidget(mTitleButton);

        mSaveDot = new QLabel(mDocumentBar);
        mSaveDot->setObjectName("cbSaveDot");
        mSaveDot->setFixedSize(9, 9);
        mSaveDot->setToolTip(tr("Sauvegarde automatique active"));
        row->addWidget(mSaveDot);
    }

    // --- En haut à droite : Fonds, Documents, Bureau, Thème, Menu ••• -----------
    mModeBar = createPill("cbModeBar", 22);
    {
        QHBoxLayout* row = new QHBoxLayout(mModeBar);
        row->setContentsMargins(8, 6, 8, 6);
        row->setSpacing(4);

        mBackgroundsButton = new QToolButton(mModeBar);
        mBackgroundsButton->setObjectName("cbTopBtn");
        mBackgroundsButton->setText(tr("Fonds"));
        mBackgroundsButton->setCheckable(true);
        mBackgroundsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        mBackgroundsButton->setIconSize(QSize(20, 20));
        mBackgroundsButton->setCursor(Qt::PointingHandCursor);
        mBackgroundsButton->setFocusPolicy(Qt::NoFocus);
        mBackgroundsButton->setToolTip(tr("Changer le fond de page (Blanc, Quadrillé, Seyès, Ardoise)"));
        connect(mBackgroundsButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::toggleBackgroundPopover);
        row->addWidget(mBackgroundsButton);

        mDocumentsButton = new QToolButton(mModeBar);
        mDocumentsButton->setObjectName("cbTopBtn");
        mDocumentsButton->setText(tr("Documents"));
        mDocumentsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        mDocumentsButton->setIconSize(QSize(20, 20));
        mDocumentsButton->setCursor(Qt::PointingHandCursor);
        mDocumentsButton->setFocusPolicy(Qt::NoFocus);
        mDocumentsButton->setToolTip(tr("Gérer mes cours et documents"));
        connect(mDocumentsButton, &QToolButton::clicked, w->actionDocument, &QAction::trigger);
        row->addWidget(mDocumentsButton);

        mDesktopButton = new QToolButton(mModeBar);
        mDesktopButton->setObjectName("cbBureauBtn");
        mDesktopButton->setText(tr("Bureau"));
        mDesktopButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        mDesktopButton->setCursor(Qt::PointingHandCursor);
        mDesktopButton->setFocusPolicy(Qt::NoFocus);
        mDesktopButton->setToolTip(tr("Annoter par-dessus Windows, un PDF ou un site"));
        connect(mDesktopButton, &QToolButton::clicked, w->actionDesktop, &QAction::trigger);
        row->addWidget(mDesktopButton);

        row->addWidget(hairlineV(mModeBar));

        mThemeButton = createIconButton(mModeBar, "moon", tr("Basculer clair / ardoise sombre"));
        connect(mThemeButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::toggleDarkMode);
        row->addWidget(mThemeButton);

        mMoreButton = createIconButton(mModeBar, "more", tr("Menu et options"));
        connect(mMoreButton, &QToolButton::clicked, this, &ClassBoardBoardChrome::showMainMenu);
        row->addWidget(mMoreButton);
    }

    // --- Barre verticale moderne à droite (ou à gauche) : 6 outils essentiels ---
    mToolRail = createPill("cbToolRail", 26);
    {
        QVBoxLayout* col = new QVBoxLayout(mToolRail);
        col->setContentsMargins(8, 10, 8, 10);
        col->setSpacing(4);

        mSelectBtn = createToolRailButton(tr("Choisir"), tr("Sélectionner et déplacer un élément"));
        mPenBtn    = createToolRailButton(tr("Écrire"), tr("Écrire au stylo"));
        mMarkerBtn = createToolRailButton(tr("Surligner"), tr("Surligner un texte ou une formule"));
        mEraserBtn = createToolRailButton(tr("Gommer"), tr("Effacer au doigt ou au stylet"));
        mTextBtn   = createToolRailButton(tr("Texte"), tr("Ajouter du texte au clavier"));
        mToolsBtn  = createToolRailButton(tr("Outils"), tr("Ouvrir les outils pédagogiques (minuteur, vocabulaire, rideau…)"));

        connect(mSelectBtn, &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Selector); });
        connect(mPenBtn,    &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Pen); });
        connect(mMarkerBtn, &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Marker); });
        connect(mEraserBtn, &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Eraser); });
        connect(mTextBtn,   &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Text); });
        connect(mToolsBtn,  &QToolButton::clicked, this, &ClassBoardBoardChrome::toggleToolsDrawer);

        col->addWidget(mSelectBtn);
        col->addWidget(mPenBtn);
        col->addWidget(mMarkerBtn);
        col->addWidget(mEraserBtn);
        col->addWidget(mTextBtn);
        col->addWidget(mToolsBtn);

        col->addSpacing(2);
        col->addWidget(hairlineH(mToolRail));
        col->addSpacing(2);

        QHBoxLayout* undoRow = new QHBoxLayout();
        undoRow->setContentsMargins(0, 0, 0, 0);
        undoRow->setSpacing(2);

        mUndoBtn = createIconButton(mToolRail, "undo", tr("Annuler (Ctrl+Z)"));
        mUndoBtn->setObjectName("cbMiniBtn");
        mUndoBtn->setFixedSize(32, 32);
        connect(mUndoBtn, &QToolButton::clicked, w->actionUndo, &QAction::trigger);
        connect(w->actionUndo, &QAction::changed, this, [this, w]() { mUndoBtn->setEnabled(w->actionUndo->isEnabled()); });
        mUndoBtn->setEnabled(w->actionUndo->isEnabled());

        mRedoBtn = createIconButton(mToolRail, "redo", tr("Rétablir (Ctrl+Y)"));
        mRedoBtn->setObjectName("cbMiniBtn");
        mRedoBtn->setFixedSize(32, 32);
        connect(mRedoBtn, &QToolButton::clicked, w->actionRedo, &QAction::trigger);
        connect(w->actionRedo, &QAction::changed, this, [this, w]() { mRedoBtn->setEnabled(w->actionRedo->isEnabled()); });
        mRedoBtn->setEnabled(w->actionRedo->isEnabled());

        undoRow->addStretch(1);
        undoRow->addWidget(mUndoBtn);
        undoRow->addWidget(mRedoBtn);
        undoRow->addStretch(1);
        col->addLayout(undoRow);
    }

    // --- Bulle contextuelle (couleurs / surligneurs / épaisseurs / tout effacer) ---
    mToolPopover = createPill("cbToolPopover", 20);
    {
        QVBoxLayout* popCol = new QVBoxLayout(mToolPopover);
        popCol->setContentsMargins(12, 10, 12, 10);
        popCol->setSpacing(8);

        mColorsRow = new QWidget(mToolPopover);
        QHBoxLayout* colorLayout = new QHBoxLayout(mColorsRow);
        colorLayout->setContentsMargins(0, 0, 0, 0);
        colorLayout->setSpacing(6);
        for (int i = 0; i < SwatchCount; ++i)
        {
            QToolButton* swatch = new QToolButton(mColorsRow);
            swatch->setObjectName("cbSwatchBtn");
            swatch->setFixedSize(34, 34);
            swatch->setIconSize(QSize(32, 32));
            swatch->setCursor(Qt::PointingHandCursor);
            swatch->setFocusPolicy(Qt::NoFocus);
            connect(swatch, &QToolButton::clicked, this, [this, i]() { selectColor(i); });
            colorLayout->addWidget(swatch);
            mColorButtons << swatch;
        }
        popCol->addWidget(mColorsRow);

        mWidthsRow = new QWidget(mToolPopover);
        QHBoxLayout* widthLayout = new QHBoxLayout(mWidthsRow);
        widthLayout->setContentsMargins(0, 0, 0, 0);
        widthLayout->setSpacing(10);
        widthLayout->addStretch(1);
        for (int i = 0; i < 3; ++i)
        {
            QToolButton* wBtn = new QToolButton(mWidthsRow);
            wBtn->setObjectName("cbWidthBtn");
            wBtn->setFixedSize(32, 32);
            wBtn->setIconSize(QSize(30, 30));
            wBtn->setCursor(Qt::PointingHandCursor);
            wBtn->setFocusPolicy(Qt::NoFocus);
            connect(wBtn, &QToolButton::clicked, this, [this, i]() { selectWidth(i); });
            widthLayout->addWidget(wBtn);
            mWidthButtons << wBtn;
        }
        widthLayout->addStretch(1);
        popCol->addWidget(mWidthsRow);

        mClearPageBtn = new QPushButton(tr("Tout effacer"), mToolPopover);
        mClearPageBtn->setObjectName("cbClearBtn");
        mClearPageBtn->setCursor(Qt::PointingHandCursor);
        mClearPageBtn->setFocusPolicy(Qt::NoFocus);
        connect(mClearPageBtn, &QPushButton::clicked, this, [this, w]() {
            w->actionClearPage->trigger();
            mToolPopover->hide();
        });
        popCol->addWidget(mClearPageBtn);
    }

    // --- En bas à gauche : zoom et déplacement toujours accessibles au tactile ---
    mZoomBar = createPill("cbZoomBar", 20);
    {
        QHBoxLayout* row = new QHBoxLayout(mZoomBar);
        row->setContentsMargins(6, 5, 6, 5);
        row->setSpacing(2);

        mZoomOutBtn = createIconButton(mZoomBar, "zoom-out", tr("Zoom arrière (réduire le document)"));
        mZoomOutBtn->setFixedSize(36, 36);
        connect(mZoomOutBtn, &QToolButton::clicked, this, []() { UBApplication::boardController->zoomOut(); });
        row->addWidget(mZoomOutBtn);

        mZoomLabel = new QLabel(mZoomBar);
        mZoomLabel->setObjectName("cbCounter");
        mZoomLabel->setAlignment(Qt::AlignCenter);
        mZoomLabel->setMinimumWidth(48);
        row->addWidget(mZoomLabel);

        mZoomInBtn = createIconButton(mZoomBar, "zoom-in", tr("Zoom avant (agrandir le document)"));
        mZoomInBtn->setFixedSize(36, 36);
        connect(mZoomInBtn, &QToolButton::clicked, this, []() { UBApplication::boardController->zoomIn(); });
        row->addWidget(mZoomInBtn);

        mFitBtn = createIconButton(mZoomBar, "fit", tr("Ajuster à 100 %"));
        mFitBtn->setFixedSize(36, 36);
        connect(mFitBtn, &QToolButton::clicked, this, []() {
            UBApplication::boardController->zoomRestore();
            UBApplication::boardController->centerRestore();
        });
        row->addWidget(mFitBtn);

        row->addWidget(hairlineV(mZoomBar));

        mHandBtn = createIconButton(mZoomBar, "hand", tr("Déplacer la page (main)"));
        mHandBtn->setCheckable(true);
        mHandBtn->setFixedSize(36, 36);
        connect(mHandBtn, &QToolButton::clicked, this, [this]() { selectTool(UBStylusTool::Hand); });
        row->addWidget(mHandBtn);
    }

    // --- En bas à droite : pagination compacte (< 1 / 1 > + Page) ---------------
    mPageBar = createPill("cbPageBar", 20);
    {
        QHBoxLayout* row = new QHBoxLayout(mPageBar);
        row->setContentsMargins(6, 5, 6, 5);
        row->setSpacing(2);

        mPrevPageBtn = createIconButton(mPageBar, "previous", tr("Page précédente"));
        mPrevPageBtn->setFixedSize(36, 36);
        connect(mPrevPageBtn, &QToolButton::clicked, w->actionBack, &QAction::trigger);
        connect(w->actionBack, &QAction::changed, this, [this, w]() { mPrevPageBtn->setEnabled(w->actionBack->isEnabled()); });
        mPrevPageBtn->setEnabled(w->actionBack->isEnabled());
        row->addWidget(mPrevPageBtn);

        mPageLabelBtn = new QToolButton(mPageBar);
        mPageLabelBtn->setObjectName("cbPageCountBtn");
        mPageLabelBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        mPageLabelBtn->setCursor(Qt::PointingHandCursor);
        mPageLabelBtn->setFocusPolicy(Qt::NoFocus);
        mPageLabelBtn->setToolTip(tr("Afficher / masquer les miniatures de pages"));
        connect(mPageLabelBtn, &QToolButton::clicked, this, []() {
            if (UBApplication::boardController && UBApplication::boardController->paletteManager())
            {
                UBLeftPalette* lp = UBApplication::boardController->paletteManager()->leftPalette();
                if (lp)
                {
                    const bool opening = !lp->isVisible() || lp->width() <= 160;
                    lp->setVisible(opening);
                    if (opening)
                    {
                        lp->showTabWidget(0);
                        lp->raise();
                    }
                }
            }
        });
        row->addWidget(mPageLabelBtn);

        mNextPageBtn = createIconButton(mPageBar, "next", tr("Page suivante"));
        mNextPageBtn->setFixedSize(36, 36);
        connect(mNextPageBtn, &QToolButton::clicked, w->actionForward, &QAction::trigger);
        connect(w->actionForward, &QAction::changed, this, [this, w]() { mNextPageBtn->setEnabled(w->actionForward->isEnabled()); });
        mNextPageBtn->setEnabled(w->actionForward->isEnabled());
        row->addWidget(mNextPageBtn);

        row->addSpacing(4);

        mNewPageBtn = new QToolButton(mPageBar);
        mNewPageBtn->setObjectName("cbNewPageBtn");
        mNewPageBtn->setText(tr("+ Page"));
        mNewPageBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        mNewPageBtn->setCursor(Qt::PointingHandCursor);
        mNewPageBtn->setFocusPolicy(Qt::NoFocus);
        mNewPageBtn->setToolTip(tr("Ajouter une nouvelle page"));
        connect(mNewPageBtn, &QToolButton::clicked, w->actionNewPage, &QAction::trigger);
        row->addWidget(mNewPageBtn);
    }

    // --- Popover moderne des fonds + Tiroir Outils pédagogiques -----------------
    mBackgroundPopover = new ClassBoardBackgroundPopover(mContainer);
    mBackgroundPopover->hide();
    connect(mBackgroundPopover, &ClassBoardBackgroundPopover::backgroundChosen, this, [this]() {
        mBackgroundsButton->setChecked(false);
        refreshColorSwatches();
    });

    mToolsDrawer = new ClassBoardToolsDrawer(mContainer);
    mToolsDrawer->hide();
    connect(mToolsDrawer, &ClassBoardToolsDrawer::closed, this, [this]() {
        mToolsBtn->setChecked(false);
    });

    // Garder tous les raccourcis clavier d'OpenBoard actifs (Ctrl+Z, flèches...)
    w->addActions(w->boardToolBar->actions());

    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &ClassBoardBoardChrome::refreshDocumentInfo);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this, &ClassBoardBoardChrome::refreshColorSwatches);
    connect(UBApplication::boardController, &UBBoardController::backgroundChanged, this, &ClassBoardBoardChrome::refreshColorSwatches);
    connect(UBApplication::boardController, &UBBoardController::zoomChanged, this, &ClassBoardBoardChrome::refreshZoom);
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this, &ClassBoardBoardChrome::stylusToolChanged);
    connect(UBDrawingController::drawingController(), &UBDrawingController::colorIndexChanged, this, &ClassBoardBoardChrome::refreshColorSwatches);
    connect(UBDrawingController::drawingController(), &UBDrawingController::colorPaletteChanged, this, &ClassBoardBoardChrome::refreshColorSwatches);
    connect(UBDrawingController::drawingController(), &UBDrawingController::lineWidthIndexChanged, this, &ClassBoardBoardChrome::refreshWidthSelection);

    QTimer* refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &ClassBoardBoardChrome::refreshDocumentInfo);
    refresh->start(1000);

    applyStyles();
    rebuildToolIcons();
    refreshDocumentInfo();
    refreshZoom();
    stylusToolChanged(UBDrawingController::drawingController()->stylusTool());
}

QFrame* ClassBoardBoardChrome::createPill(const QString& name, int radius)
{
    Q_UNUSED(radius);
    QFrame* pill = new QFrame(mContainer);
    pill->setObjectName(name);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(pill);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(15, 23, 42, ClassBoardTheme::isDark() ? 115 : 30));
    pill->setGraphicsEffect(shadow);

    return pill;
}

QToolButton* ClassBoardBoardChrome::createToolRailButton(const QString& label, const QString& tooltip)
{
    QToolButton* btn = new QToolButton(mToolRail);
    btn->setObjectName("cbRailBtn");
    btn->setCheckable(true);
    btn->setText(label);
    btn->setToolTip(tooltip);
    btn->setAccessibleName(label);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setToolButtonStyle(ClassBoardProfile::showToolLabels() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    return btn;
}

QToolButton* ClassBoardBoardChrome::createIconButton(QWidget* parent, const QString& iconName, const QString& tooltip)
{
    QToolButton* btn = new QToolButton(parent);
    btn->setObjectName("cbIconBtn");
    btn->setIcon(ClassBoardIcons::icon(iconName, ClassBoardIcons::Style::Toolbar));
    btn->setIconSize(QSize(20, 20));
    btn->setFixedSize(38, 38);
    btn->setToolTip(tooltip);
    btn->setAccessibleName(tooltip);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    return btn;
}

void ClassBoardBoardChrome::applyStyles()
{
    const ClassBoardTheme::Colors c = ClassBoardTheme::colors();
    const bool dark = ClassBoardTheme::isDark();
    const QString surface = ClassBoardTheme::colorName(c.surface);
    const QString border = ClassBoardTheme::colorName(c.border);
    const QString text = ClassBoardTheme::colorName(c.text);
    const QString muted = ClassBoardTheme::colorName(c.textSecondary);
    const QString hover = ClassBoardTheme::colorName(c.surfaceAlt);
    const QString accent = ClassBoardTheme::colorName(c.accent);
    const QString accentSoft = ClassBoardTheme::colorName(c.accentSoft);
    const QString primaryBg = dark ? QStringLiteral("#3B82F6") : QStringLiteral("#4F46E5");
    const QString primaryHover = dark ? QStringLiteral("#2563EB") : QStringLiteral("#4338CA");
    const QString clearBg = dark ? QStringLiteral("rgba(251, 113, 133, 36)") : QStringLiteral("#FFF1F2");
    const QString clearFg = dark ? QStringLiteral("#FDA4AF") : QStringLiteral("#E11D48");

    const QString sharedPill = QStringLiteral(R"(
        QFrame#cbDocumentBar, QFrame#cbModeBar, QFrame#cbPageBar, QFrame#cbZoomBar {
            background: %1; border: 1px solid %2; border-radius: 22px;
        }
        QFrame#cbToolRail {
            background: %1; border: 1px solid %2; border-radius: 26px;
        }
        QFrame#cbToolPopover {
            background: %1; border: 1px solid %2; border-radius: 20px;
        }
        QLabel { background: transparent; color: %3; }
        QLabel#cbTitle { font-size: 14px; font-weight: 700; color: %3; }
        QLabel#cbSaveDot { background: #10B981; border-radius: 4px; }
        QLabel#cbCounter { font-size: 13px; font-weight: 700; color: %3; }
        QFrame#cbDividerH { background: %2; min-height: 1px; max-height: 1px; margin: 2px 6px; }
        QFrame#cbDividerV { background: %2; min-width: 1px; max-width: 1px; margin: 6px 4px; }

        QToolButton#cbLogoBtn {
            background: transparent; border: none; border-radius: 12px; padding: 2px;
        }
        QToolButton#cbLogoBtn:hover { background: %5; }

        QToolButton#cbTitleBtn {
            background: transparent; border: none; border-radius: 12px; color: %3;
            font-size: 14px; font-weight: 700; padding: 6px 10px;
        }
        QToolButton#cbTitleBtn:hover { background: %5; }

        QToolButton#cbTopBtn {
            background: transparent; border: none; border-radius: 14px; color: %3;
            font-size: 13px; font-weight: 600; padding: 8px 12px; min-height: 22px;
        }
        QToolButton#cbTopBtn:hover { background: %5; }
        QToolButton#cbTopBtn:checked { background: %7; color: %6; }

        QToolButton#cbBureauBtn {
            background: %8; border: none; border-radius: 16px; color: #FFFFFF;
            font-size: 13px; font-weight: 700; padding: 8px 16px; min-height: 22px;
        }
        QToolButton#cbBureauBtn:hover { background: %9; }

        QToolButton#cbIconBtn, QToolButton#cbMiniBtn {
            background: transparent; border: none; border-radius: 14px; color: %3;
        }
        QToolButton#cbIconBtn:hover, QToolButton#cbMiniBtn:hover { background: %5; }
        QToolButton#cbIconBtn:checked { background: %7; color: %6; }
        QToolButton#cbIconBtn:disabled, QToolButton#cbMiniBtn:disabled { color: %4; }

        QToolButton#cbRailBtn {
            background: transparent; border: none; border-radius: 16px; color: %3;
            font-size: 11px; font-weight: 700; padding: 6px 4px;
        }
        QToolButton#cbRailBtn:hover { background: %5; }
        QToolButton#cbRailBtn:checked { background: %7; color: %6; }

        QToolButton#cbSwatchBtn, QToolButton#cbWidthBtn {
            background: transparent; border: none; border-radius: 16px; padding: 0;
        }
        QToolButton#cbSwatchBtn:hover, QToolButton#cbWidthBtn:hover { background: %5; }

        QPushButton#cbClearBtn {
            background: %10; color: %11; border: none; border-radius: 12px;
            font-size: 12px; font-weight: 700; padding: 7px 14px; min-height: 28px;
        }
        QPushButton#cbClearBtn:hover { background: %11; color: #FFFFFF; }

        QToolButton#cbPageCountBtn {
            background: transparent; border: none; border-radius: 12px; color: %3;
            font-size: 13px; font-weight: 700; padding: 6px 10px; min-width: 52px;
        }
        QToolButton#cbPageCountBtn:hover { background: %5; }

        QToolButton#cbNewPageBtn {
            background: %7; border: none; border-radius: 14px; color: %6;
            font-size: 13px; font-weight: 700; padding: 8px 14px;
        }
        QToolButton#cbNewPageBtn:hover { background: %8; color: #FFFFFF; }
    )").arg(surface, border, text, muted, hover, accent, accentSoft, primaryBg, primaryHover)
       .arg(clearBg, clearFg);

    for (QFrame* pill : {mDocumentBar, mModeBar, mToolRail, mToolPopover, mZoomBar, mPageBar})
    {
        pill->setStyleSheet(sharedPill);
        if (auto* shadow = qobject_cast<QGraphicsDropShadowEffect*>(pill->graphicsEffect()))
            shadow->setColor(QColor(15, 23, 42, dark ? 115 : 30));
    }

    if (mBackgroundPopover)
        mBackgroundPopover->applyTheme();
    if (mToolsDrawer)
        mToolsDrawer->applyTheme();
}

void ClassBoardBoardChrome::rebuildToolIcons()
{
    const bool dark = ClassBoardTheme::isDark();
    const bool showLabels = ClassBoardProfile::showToolLabels();
    const int scalePct = ClassBoardProfile::toolbarScale();
    const int iconPx = qBound(34, qRound(38 * scalePct / 125.0), 46);
    const int btnW = qBound(58, qRound(68 * scalePct / 125.0), 78);
    const int btnH = showLabels ? qBound(58, qRound(66 * scalePct / 125.0), 76) : btnW;

    auto setRailIcon = [&](QToolButton* btn, const QString& iconName,
                           const QColor& fg, const QColor& bg,
                           const QColor& activeFg, const QColor& activeBg)
    {
        QColor nFg = dark ? fg.lighter(135) : fg;
        QColor nBg = dark ? QColor(fg.red(), fg.green(), fg.blue(), 52) : bg;
        btn->setIcon(ClassBoardIcons::duotoneIcon(iconName, nFg, nBg, activeFg, activeBg));
        btn->setIconSize(QSize(iconPx, iconPx));
        btn->setFixedSize(btnW, btnH);
        btn->setToolButtonStyle(showLabels ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
    };

    setRailIcon(mSelectBtn, "selector", QColor("#4F46E5"), QColor("#EEF2FF"), QColor("#FFFFFF"), QColor("#4F46E5"));
    setRailIcon(mPenBtn,    "pen",      QColor("#2563EB"), QColor("#DBEAFE"), QColor("#FFFFFF"), QColor("#2563EB"));
    setRailIcon(mMarkerBtn, "marker",   QColor("#B45309"), QColor("#FEF3C7"), QColor("#78350F"), QColor("#FDE047"));
    setRailIcon(mEraserBtn, "eraser",   QColor("#E11D48"), QColor("#FFE4E6"), QColor("#FFFFFF"), QColor("#E11D48"));
    setRailIcon(mTextBtn,   "text",     QColor("#475569"), QColor("#F1F5F9"), QColor("#FFFFFF"), QColor("#475569"));
    setRailIcon(mToolsBtn,  "sparkles", QColor("#7C3AED"), QColor("#F3E8FF"), QColor("#FFFFFF"), QColor("#6D28D9"));

    mBackgroundsButton->setIcon(ClassBoardIcons::icon("backgrounds", ClassBoardIcons::Style::Toolbar));
    mDocumentsButton->setIcon(ClassBoardIcons::icon("documents", ClassBoardIcons::Style::Toolbar));
    mThemeButton->setIcon(ClassBoardIcons::icon(dark ? "sun" : "moon", ClassBoardIcons::Style::Toolbar));
    mThemeButton->setToolTip(dark ? tr("Passer en fond clair") : tr("Passer en mode ardoise sombre"));
    mMoreButton->setIcon(ClassBoardIcons::icon("more", ClassBoardIcons::Style::Toolbar));
    mUndoBtn->setIcon(ClassBoardIcons::icon("undo", ClassBoardIcons::Style::Toolbar));
    mRedoBtn->setIcon(ClassBoardIcons::icon("redo", ClassBoardIcons::Style::Toolbar));
    mZoomOutBtn->setIcon(ClassBoardIcons::icon("zoom-out", ClassBoardIcons::Style::Toolbar));
    mZoomInBtn->setIcon(ClassBoardIcons::icon("zoom-in", ClassBoardIcons::Style::Toolbar));
    mFitBtn->setIcon(ClassBoardIcons::icon("fit", ClassBoardIcons::Style::Toolbar));
    mHandBtn->setIcon(ClassBoardIcons::icon("hand", ClassBoardIcons::Style::Toolbar));
    mPrevPageBtn->setIcon(ClassBoardIcons::icon("previous", ClassBoardIcons::Style::Toolbar));
    mNextPageBtn->setIcon(ClassBoardIcons::icon("next", ClassBoardIcons::Style::Toolbar));

    refreshColorSwatches();
    refreshWidthSelection();
}

void ClassBoardBoardChrome::setInkAnchor(QWidget* anchor)
{
    Q_UNUSED(anchor);
}

bool ClassBoardBoardChrome::eventFilter(QObject* watched, QEvent* event)
{
    return QObject::eventFilter(watched, event);
}

void ClassBoardBoardChrome::place(const QRect& area)
{
    mArea = area;
    if (!mArea.isValid())
        return;

    for (QFrame* pill : {mDocumentBar, mModeBar, mToolRail, mToolPopover, mZoomBar, mPageBar})
        pill->adjustSize();

    mDocumentBar->move(area.left() + Margin, area.top() + Margin);
    mModeBar->move(area.right() - Margin - mModeBar->width(), area.top() + Margin);
    mZoomBar->move(area.left() + Margin, area.bottom() - Margin - mZoomBar->height());
    mPageBar->move(area.right() - Margin - mPageBar->width(), area.bottom() - Margin - mPageBar->height());

    // Barre verticale à droite par défaut (ou à gauche si choisi dans le profil)
    const bool onLeft = ClassBoardProfile::toolbarSide() == ClassBoardProfile::ToolbarSide::Left;
    const int railX = onLeft ? (area.left() + Margin) : (area.right() - Margin - mToolRail->width());
    const int minRailY = mModeBar->geometry().bottom() + 12;
    const int maxRailY = mPageBar->geometry().top() - 12 - mToolRail->height();
    const int centeredY = area.top() + (area.height() - mToolRail->height()) / 2;
    const int railY = qBound(minRailY, centeredY, qMax(minRailY, maxRailY));
    mToolRail->move(railX, railY);

    placeToolPopover();
    placeBackgroundPopover();
    placeToolsDrawer();
    raiseAll();
}

void ClassBoardBoardChrome::placeToolPopover()
{
    if (!mToolPopover || !mToolRail)
        return;

    mToolPopover->adjustSize();
    const bool onLeft = ClassBoardProfile::toolbarSide() == ClassBoardProfile::ToolbarSide::Left;

    QToolButton* anchorBtn = mPenBtn;
    if (mCurrentTool == UBStylusTool::Marker)
        anchorBtn = mMarkerBtn;
    else if (mCurrentTool == UBStylusTool::Eraser)
        anchorBtn = mEraserBtn;

    const QPoint btnCenter = anchorBtn->mapTo(mContainer, QPoint(anchorBtn->width() / 2, anchorBtn->height() / 2));
    const int x = onLeft
        ? (mToolRail->geometry().right() + 12)
        : (mToolRail->geometry().left() - 12 - mToolPopover->width());
    int y = btnCenter.y() - mToolPopover->height() / 2;
    y = qBound(mArea.top() + TopReservedHeight, y,
               qMax(mArea.top() + TopReservedHeight, mArea.bottom() - Margin - mToolPopover->height()));

    mToolPopover->move(x, y);
    mToolPopover->raise();
}

void ClassBoardBoardChrome::placeBackgroundPopover()
{
    if (!mBackgroundPopover || !mModeBar)
        return;

    mBackgroundPopover->adjustSize();
    const int maxX = mToolRail ? (mToolRail->geometry().left() - 12 - mBackgroundPopover->width())
                               : (mArea.right() - Margin - mBackgroundPopover->width());
    const int x = qBound(mArea.left() + Margin, mModeBar->geometry().left(), qMax(mArea.left() + Margin, maxX));
    const int y = mModeBar->geometry().bottom() + 10;
    mBackgroundPopover->move(x, y);
    mBackgroundPopover->raise();
}

void ClassBoardBoardChrome::placeToolsDrawer()
{
    if (!mToolsDrawer || !mToolRail)
        return;

    const int maxH = qMax(320, mArea.height() - TopReservedHeight - Margin * 2 - 24);
    mToolsDrawer->setFixedHeight(qMin(540, maxH));

    const bool onLeft = ClassBoardProfile::toolbarSide() == ClassBoardProfile::ToolbarSide::Left;
    const int x = onLeft
        ? (mToolRail->geometry().right() + 14)
        : (mToolRail->geometry().left() - 14 - mToolsDrawer->width());
    int y = mToolRail->geometry().bottom() - mToolsDrawer->height();
    y = qBound(mArea.top() + TopReservedHeight, y,
               qMax(mArea.top() + TopReservedHeight, mArea.bottom() - Margin - mToolsDrawer->height()));

    mToolsDrawer->move(x, y);
    mToolsDrawer->raise();
}

QRect ClassBoardBoardChrome::leftFreeArea() const
{
    const int top = mArea.top() + TopReservedHeight;
    const int bottom = mArea.bottom() - Margin;
    return QRect(mArea.left(), top, mArea.width(), qMax(0, bottom - top));
}

void ClassBoardBoardChrome::setVisible(bool visible)
{
    mVisible = visible;

    for (QFrame* pill : {mDocumentBar, mModeBar, mToolRail, mZoomBar, mPageBar})
        pill->setVisible(visible);

    if (!visible)
    {
        closePopovers();
        mToolPopover->hide();
    }
    else
    {
        refreshZoom();
        stylusToolChanged(UBDrawingController::drawingController()->stylusTool());
    }
}

void ClassBoardBoardChrome::reloadFromProfile()
{
    ClassBoardTheme::apply();
    ClassBoardIcons::applyToMainWindow(UBApplication::mainWindow);
    applyStyles();
    rebuildToolIcons();
    if (mToolsDrawer)
        mToolsDrawer->reloadForProfile();
    if (mArea.isValid())
        place(mArea);
}

void ClassBoardBoardChrome::raiseAll()
{
    for (QFrame* pill : {mDocumentBar, mModeBar, mToolRail, mZoomBar, mPageBar, mToolPopover})
        pill->raise();
    if (mBackgroundPopover && mBackgroundPopover->isVisible())
        mBackgroundPopover->raise();
    if (mToolsDrawer && mToolsDrawer->isVisible())
        mToolsDrawer->raise();
}

void ClassBoardBoardChrome::closePopovers()
{
    if (mBackgroundPopover)
        mBackgroundPopover->hide();
    if (mBackgroundsButton)
        mBackgroundsButton->setChecked(false);
    if (mToolsDrawer)
        mToolsDrawer->hide();
    if (mToolsBtn)
        mToolsBtn->setChecked(false);
}

void ClassBoardBoardChrome::refreshDocumentInfo()
{
    UBBoardController* board = UBApplication::boardController;
    if (!board || !board->selectedDocument())
        return;

    const QString name = board->selectedDocument()->name();
    const QString display = name.isEmpty() ? tr("Nouveau cours") : name;
    if (mTitleButton->text() != display)
    {
        mTitleButton->setText(display);
        mDocumentBar->adjustSize();
    }

    const int count = qMax(1, board->selectedDocument()->pageCount());
    const QString pages = QString("%1 / %2").arg(board->activeSceneIndex() + 1).arg(count);
    if (mPageLabelBtn->text() != pages)
    {
        mPageLabelBtn->setText(pages);
        if (mArea.isValid())
            place(mArea);
    }
}

void ClassBoardBoardChrome::markSaved()
{
    mSaveDot->setToolTip(tr("Enregistré automatiquement · %1").arg(QTime::currentTime().toString("HH:mm")));
}

void ClassBoardBoardChrome::refreshZoom()
{
    const qreal zoom = UBApplication::boardController ? UBApplication::boardController->currentZoom() : 1.0;
    const int pct = qRound(zoom * 100);
    mZoomLabel->setText(QString("%1 %").arg(pct));
    mZoomBar->setVisible(mVisible);
    if (mZoomBar->isVisible() && mArea.isValid())
    {
        mZoomBar->adjustSize();
        mZoomBar->move(mArea.left() + Margin, mArea.bottom() - Margin - mZoomBar->height());
    }
}

void ClassBoardBoardChrome::selectTool(int tool)
{
    closePopovers();
    if (mCurrentTool == tool && (tool == UBStylusTool::Pen || tool == UBStylusTool::Marker || tool == UBStylusTool::Eraser))
    {
        mPopoverSuppressed = !mPopoverSuppressed;
        mToolPopover->setVisible(mVisible && !mPopoverSuppressed);
        if (mToolPopover->isVisible())
            placeToolPopover();
        return;
    }

    mPopoverSuppressed = false;
    UBDrawingController::drawingController()->setStylusTool(tool);
}

void ClassBoardBoardChrome::stylusToolChanged(int tool)
{
    mCurrentTool = tool;

    mSelectBtn->setChecked(tool == UBStylusTool::Selector);
    mPenBtn->setChecked(tool == UBStylusTool::Pen || tool == UBStylusTool::Line);
    mMarkerBtn->setChecked(tool == UBStylusTool::Marker);
    mEraserBtn->setChecked(tool == UBStylusTool::Eraser);
    mTextBtn->setChecked(tool == UBStylusTool::Text);
    if (mHandBtn)
        mHandBtn->setChecked(tool == UBStylusTool::Hand);

    const bool drawing = (tool == UBStylusTool::Pen || tool == UBStylusTool::Marker || tool == UBStylusTool::Line);
    const bool erasing = (tool == UBStylusTool::Eraser);

    mColorsRow->setVisible(drawing);
    mWidthsRow->setVisible(drawing || erasing);
    mClearPageBtn->setVisible(erasing);

    refreshColorSwatches();
    refreshWidthSelection();

    mToolPopover->setVisible(mVisible && !mPopoverSuppressed && (drawing || erasing));
    if (mToolPopover->isVisible())
        placeToolPopover();
}

void ClassBoardBoardChrome::refreshColorSwatches()
{
    const bool darkScene = UBApplication::boardController && UBApplication::boardController->activeScene()
                           && UBApplication::boardController->activeScene()->isDarkBackground();
    const bool isMarker = (mCurrentTool == UBStylusTool::Marker);
    const QList<QColor> colors = isMarker
        ? UBSettings::settings()->markerColors(darkScene)
        : UBSettings::settings()->penColors(darkScene);
    const int activeIdx = isMarker
        ? UBSettings::settings()->markerColorIndex()
        : UBSettings::settings()->penColorIndex();
    const bool darkUi = ClassBoardTheme::isDark();

    for (int i = 0; i < mColorButtons.size(); ++i)
    {
        QToolButton* btn = mColorButtons.at(i);
        if (i < colors.size())
        {
            btn->setVisible(true);
            btn->setIcon(swatchIcon(colors.at(i), i == activeIdx, darkUi));
        }
        else
        {
            btn->setVisible(false);
        }
    }
}

void ClassBoardBoardChrome::refreshWidthSelection()
{
    const bool erasing = (mCurrentTool == UBStylusTool::Eraser);
    const int activeIdx = erasing
        ? UBSettings::settings()->eraserWidthIndex()
        : UBDrawingController::drawingController()->currentToolWidthIndex();
    const bool darkUi = ClassBoardTheme::isDark();

    for (int i = 0; i < mWidthButtons.size(); ++i)
        mWidthButtons.at(i)->setIcon(dotWidthIcon(i, i == activeIdx, erasing, darkUi));
}

void ClassBoardBoardChrome::selectColor(int index)
{
    if (UBApplication::boardController)
        UBApplication::boardController->setColorIndex(index);
    refreshColorSwatches();
}

void ClassBoardBoardChrome::selectWidth(int index)
{
    if (mCurrentTool == UBStylusTool::Eraser)
        UBDrawingController::drawingController()->setEraserWidthIndex(index);
    else
        UBDrawingController::drawingController()->setLineWidthIndex(index);
    refreshWidthSelection();
}

void ClassBoardBoardChrome::toggleBackgroundPopover()
{
    const bool open = mBackgroundPopover->isHidden();
    closePopovers();
    if (open)
    {
        mBackgroundPopover->syncSelection();
        mBackgroundPopover->show();
        mBackgroundsButton->setChecked(true);
        placeBackgroundPopover();
    }
}

void ClassBoardBoardChrome::toggleToolsDrawer()
{
    const bool open = mToolsDrawer->isHidden();
    closePopovers();
    if (open)
    {
        mToolPopover->hide();
        mToolsDrawer->show();
        mToolsBtn->setChecked(true);
        placeToolsDrawer();
    }
}

void ClassBoardBoardChrome::toggleDarkMode()
{
    closePopovers();
    const bool nextDark = !ClassBoardTheme::isDark();
    ClassBoardProfile::setTheme(nextDark ? ClassBoardProfile::Theme::Dark : ClassBoardProfile::Theme::Light);

    // Basculer aussi le fond de page courant (blanc <-> ardoise) en gardant le motif (uni/quadrillé/Seyès)
    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        const UBPageBackground kind = UBApplication::boardController->activeScene()->pageBackground();
        UBApplication::boardController->changeBackground(nextDark, kind);
    }

    reloadFromProfile();
}

void ClassBoardBoardChrome::renameCurrentDocument()
{
    closePopovers();
    UBBoardController* board = UBApplication::boardController;
    if (!board || !board->selectedDocument())
        return;

    auto doc = board->selectedDocument();
    bool ok = false;
    const QString name = QInputDialog::getText(
        UBApplication::mainWindow,
        tr("Renommer ce cours"),
        tr("Nom du cours (ex. Anglais 4e B — Unit 3) :"),
        QLineEdit::Normal,
        doc->name(),
        &ok).trimmed();

    if (!ok || name.isEmpty() || name == doc->name())
        return;

    doc->setMetaData(UBSettings::documentName, name);
    UBPersistenceManager::persistenceManager()->persistDocumentMetadata(doc);
    refreshDocumentInfo();
}

void ClassBoardBoardChrome::exportCurrentDocumentPdf()
{
    closePopovers();
    UBBoardController* board = UBApplication::boardController;
    if (!board || !board->selectedDocument())
        return;

    board->persistCurrentScene(false, true);
    UBExportPDF exporter;
    exporter.persist(board->selectedDocument());
}

void ClassBoardBoardChrome::showMainMenu()
{
    closePopovers();

    QMenu menu(mContainer);
    UBMainWindow* w = UBApplication::mainWindow;

    QAction* renameAct = menu.addAction(ClassBoardIcons::icon("rename", ClassBoardIcons::Style::Toolbar), tr("Renommer ce cours…"));
    connect(renameAct, &QAction::triggered, this, &ClassBoardBoardChrome::renameCurrentDocument);

    QAction* exportPdfAct = menu.addAction(ClassBoardIcons::icon("export", ClassBoardIcons::Style::Toolbar), tr("Exporter en PDF (Pronote / ENT)…"));
    connect(exportPdfAct, &QAction::triggered, this, &ClassBoardBoardChrome::exportCurrentDocumentPdf);

    menu.addSeparator();

    QAction* profileAct = menu.addAction(QIcon(":/images/ClassBoard.png"), tr("Mes matières et préférences…"));
    connect(profileAct, &QAction::triggered, this, []() {
        if (UBApplication::boardController && UBApplication::boardController->paletteManager())
            UBApplication::boardController->paletteManager()->showClassBoardProfile();
    });

    const bool onLeft = ClassBoardProfile::toolbarSide() == ClassBoardProfile::ToolbarSide::Left;
    QAction* sideAct = menu.addAction(onLeft ? tr("Mettre la barre d'outils à droite")
                                             : tr("Mettre la barre d'outils à gauche"));
    connect(sideAct, &QAction::triggered, this, [this, onLeft]() {
        ClassBoardProfile::setToolbarSide(onLeft ? ClassBoardProfile::ToolbarSide::Right
                                                 : ClassBoardProfile::ToolbarSide::Left);
        if (mArea.isValid())
            place(mArea);
    });

    menu.addSeparator();

    QAction* dupAct = menu.addAction(tr("Dupliquer cette page"));
    connect(dupAct, &QAction::triggered, w->actionDuplicatePage, &QAction::trigger);

    QAction* webAct = menu.addAction(tr("Navigateur Web intégré"));
    connect(webAct, &QAction::triggered, w->actionWeb, &QAction::trigger);

    menu.addSeparator();

    QAction* prefsAct = menu.addAction(tr("Réglages avancés OpenBoard…"));
    connect(prefsAct, &QAction::triggered, w->actionPreferences, &QAction::trigger);

    QAction* quitAct = menu.addAction(tr("Quitter ClassBoard"));
    connect(quitAct, &QAction::triggered, w->actionQuit, &QAction::trigger);

    QWidget* anchor = (sender() == mMenuButton) ? static_cast<QWidget*>(mMenuButton) : static_cast<QWidget*>(mMoreButton);
    QPoint pos = anchor->mapToGlobal(QPoint(0, anchor->height() + 8));
    if (anchor == mMoreButton)
        pos.setX(anchor->mapToGlobal(QPoint(anchor->width(), 0)).x() - menu.sizeHint().width());

    menu.exec(pos);
}
