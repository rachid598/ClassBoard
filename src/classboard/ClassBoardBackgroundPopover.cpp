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

#include "ClassBoardBackgroundPopover.h"

#include "ClassBoardTheme.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "domain/UBGraphicsScene.h"

#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QToolButton>
#include <QVBoxLayout>

ClassBoardBackgroundPopover::ClassBoardBackgroundPopover(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("cbBackgroundPopover");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 16);
    root->setSpacing(10);

    QLabel* title = new QLabel(tr("Fond de page"), this);
    title->setObjectName("cbPopTitle");
    root->addWidget(title);

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(10);

    struct Def { bool dark; UBPageBackground kind; QString label; };
    const QList<Def> defs = {
        {false, UBPageBackground::plain,   tr("Blanc")},
        {false, UBPageBackground::crossed, tr("Quadrillé")},
        {false, UBPageBackground::ruled,   tr("Seyès")},
        {true,  UBPageBackground::plain,   tr("Ardoise")},
        {true,  UBPageBackground::crossed, tr("Quadr. noir")},
        {true,  UBPageBackground::ruled,   tr("Seyès noir")},
    };

    for (int i = 0; i < defs.size(); ++i)
    {
        const Def& d = defs.at(i);
        QToolButton* btn = new QToolButton(this);
        btn->setObjectName("cbBgCard");
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setIcon(buildPreviewIcon(d.dark, static_cast<int>(d.kind)));
        btn->setIconSize(QSize(88, 54));
        btn->setText(d.label);
        btn->setFixedSize(112, 86);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::NoFocus);

        const bool dark = d.dark;
        const int kind = static_cast<int>(d.kind);
        connect(btn, &QToolButton::clicked, this, [this, dark, kind]() {
            choose(dark, kind);
        });

        grid->addWidget(btn, i / 3, i % 3);
        mOptions << Option{dark, kind, btn};
    }

    root->addLayout(grid);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(32);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(15, 23, 42, 42));
    setGraphicsEffect(shadow);

    applyTheme();
    adjustSize();
}

void ClassBoardBackgroundPopover::applyTheme()
{
    const ClassBoardTheme::Colors c = ClassBoardTheme::colors();
    setStyleSheet(QStringLiteral(R"(
        QFrame#cbBackgroundPopover {
            background: %1; border: 1px solid %2; border-radius: 22px;
        }
        QFrame#cbBackgroundPopover QLabel#cbPopTitle {
            background: transparent; color: %3; font-size: 14px; font-weight: 700;
        }
        QFrame#cbBackgroundPopover QToolButton#cbBgCard {
            background: transparent; color: %3; border: 2px solid transparent;
            border-radius: 14px; font-size: 12px; font-weight: 600; padding: 4px;
        }
        QFrame#cbBackgroundPopover QToolButton#cbBgCard:hover {
            background: %4;
        }
        QFrame#cbBackgroundPopover QToolButton#cbBgCard:checked {
            background: %5; border: 2px solid %6; color: %6;
        }
    )").arg(ClassBoardTheme::colorName(c.surface),
            ClassBoardTheme::colorName(c.border),
            ClassBoardTheme::colorName(c.text),
            ClassBoardTheme::colorName(c.surfaceAlt),
            ClassBoardTheme::colorName(c.accentSoft),
            ClassBoardTheme::colorName(c.accent)));
}

QIcon ClassBoardBackgroundPopover::buildPreviewIcon(bool dark, int kind) const
{
    QIcon icon;
    for (int scale : {1, 2})
    {
        const int w = 84 * scale;
        const int h = 54 * scale;
        QPixmap pm(w, h);
        pm.fill(Qt::transparent);

        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);

        QRectF rect(1.5 * scale, 1.5 * scale, w - 3 * scale, h - 3 * scale);
        QPainterPath clip;
        clip.addRoundedRect(rect, 9 * scale, 9 * scale);
        p.save();
        p.setClipPath(clip);

        // Fond papier ou ardoise
        p.fillRect(rect, dark ? QColor("#1E262C") : QColor("#FFFFFF"));

        const UBPageBackground bg = static_cast<UBPageBackground>(kind);
        if (bg == UBPageBackground::crossed)
        {
            p.setPen(QPen(dark ? QColor(148, 163, 184, 55) : QColor("#CBD5E1"), 1.0 * scale));
            const int step = 12 * scale;
            for (int x = step; x < w; x += step)
                p.drawLine(x, 0, x, h);
            for (int y = step; y < h; y += step)
                p.drawLine(0, y, w, y);
        }
        else if (bg == UBPageBackground::ruled)
        {
            // Seyès : lignes fines bleu clair + ligne de base violette + marge rouge discrète
            p.setPen(QPen(dark ? QColor(125, 211, 252, 45) : QColor("#CFE8FF"), 0.8 * scale));
            for (int y = 6 * scale; y < h; y += 5 * scale)
                p.drawLine(0, y, w, y);
            p.setPen(QPen(dark ? QColor(167, 139, 250, 95) : QColor("#9C8AE8"), 1.2 * scale));
            for (int y = 16 * scale; y < h; y += 15 * scale)
                p.drawLine(0, y, w, y);
            p.setPen(QPen(dark ? QColor(251, 113, 133, 85) : QColor("#F5A3B5"), 1.1 * scale));
            p.drawLine(14 * scale, 0, 14 * scale, h);
        }

        p.restore();
        p.setPen(QPen(dark ? QColor("#3A4854") : QColor("#DCE2EA"), 1.2 * scale));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect, 9 * scale, 9 * scale);
        p.end();

        pm.setDevicePixelRatio(scale);
        icon.addPixmap(pm);
    }
    return icon;
}

void ClassBoardBackgroundPopover::syncSelection()
{
    if (!UBApplication::boardController || !UBApplication::boardController->activeScene())
        return;

    const auto scene = UBApplication::boardController->activeScene();
    const bool dark = scene->isDarkBackground();
    const int kind = static_cast<int>(scene->pageBackground());

    for (const Option& o : mOptions)
    {
        if (o.button)
            o.button->setChecked(o.dark == dark && o.kind == kind);
    }
}

void ClassBoardBackgroundPopover::choose(bool dark, int kind)
{
    if (UBApplication::boardController)
        UBApplication::boardController->changeBackground(dark, static_cast<UBPageBackground>(kind));

    syncSelection();
    hide();
    emit backgroundChosen();
}
