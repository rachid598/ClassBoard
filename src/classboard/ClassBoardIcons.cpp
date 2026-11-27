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

#include "ClassBoardIcons.h"

#include "ClassBoardTheme.h"

#include "gui/UBMainWindow.h"

#include <QFile>
#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QSvgRenderer>

namespace
{
    const int CanvasSize = 96;

    QByteArray svgData(const QString& name, const QColor& color)
    {
        QFile file(QStringLiteral(":/classboard/tools/%1.svg").arg(name));

        if (!file.open(QIODevice::ReadOnly))
            return QByteArray();

        QByteArray svg = file.readAll();
        svg.replace("currentColor", color.name().toLatin1());
        return svg;
    }

    void drawGlyph(QPainter& painter, const QString& name, const QColor& color, const QRectF& rect)
    {
        QSvgRenderer renderer(svgData(name, color));
        renderer.render(&painter, rect);
    }

    void drawMenuArrow(QPainter& painter, const QColor& color)
    {
        QPainterPath arrow;
        arrow.moveTo(CanvasSize * 0.78, CanvasSize * 0.86);
        arrow.lineTo(CanvasSize * 0.94, CanvasSize * 0.86);
        arrow.lineTo(CanvasSize * 0.86, CanvasSize * 0.95);
        arrow.closeSubpath();
        painter.fillPath(arrow, color);
    }

    QPixmap render(const QString& name, ClassBoardIcons::Style style, bool on, bool disabled, bool withMenuArrow,
                   const QColor& forcedColor)
    {
        const ClassBoardTheme::Colors c = ClassBoardTheme::colors();

        QPixmap pixmap(CanvasSize, CanvasSize);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QColor glyph = c.text;
        qreal glyphRatio = 0.62;

        switch (style)
        {
        case ClassBoardIcons::Style::Tool:
            if (on)
            {
                painter.setPen(Qt::NoPen);
                painter.setBrush(c.accentSoft);
                painter.drawRoundedRect(QRectF(2, 2, CanvasSize - 4, CanvasSize - 4), 22, 22);
                glyph = c.accent;
            }
            break;

        case ClassBoardIcons::Style::Favorite:
            painter.setPen(QPen(c.border, 2));
            painter.setBrush(c.surfaceAlt);
            painter.drawRoundedRect(QRectF(3, 3, CanvasSize - 6, CanvasSize - 6), 22, 22);
            glyphRatio = 0.58;
            break;

        case ClassBoardIcons::Style::Toolbar:
            glyphRatio = 0.78;
            break;
        }

        if (forcedColor.isValid() && !on)
            glyph = forcedColor;

        if (disabled)
        {
            glyph = c.textSecondary;
            glyph.setAlpha(110);
        }

        const qreal size = CanvasSize * glyphRatio;
        const qreal offset = (CanvasSize - size) / 2.0;
        drawGlyph(painter, name, glyph, QRectF(offset, offset, size, size));

        if (withMenuArrow)
            drawMenuArrow(painter, glyph);

        painter.end();
        return pixmap;
    }
}

QIcon ClassBoardIcons::icon(const QString& name, Style style, bool withMenuArrow, const QColor& color)
{
    QIcon icon;
    icon.addPixmap(render(name, style, false, false, withMenuArrow, color), QIcon::Normal, QIcon::Off);
    icon.addPixmap(render(name, style, true, false, withMenuArrow, color), QIcon::Normal, QIcon::On);
    icon.addPixmap(render(name, style, false, true, withMenuArrow, color), QIcon::Disabled, QIcon::Off);
    icon.addPixmap(render(name, style, false, true, withMenuArrow, color), QIcon::Disabled, QIcon::On);
    return icon;
}

QIcon ClassBoardIcons::duotoneIcon(const QString& name, const QColor& fgColor, const QColor& bgColor,
                                   const QColor& activeFg, const QColor& activeBg)
{
    const QString resource = QStringLiteral(":/classboard/tools/%1.svg").arg(name);
    const QIcon offIcon = ClassBoardTheme::duotoneIcon(resource, fgColor, bgColor, 44, 13);
    const QIcon onIcon = ClassBoardTheme::duotoneIcon(resource,
                                                      activeFg.isValid() ? activeFg : fgColor,
                                                      activeBg.isValid() ? activeBg : bgColor,
                                                      44, 13);

    QIcon icon;
    for (const QSize& s : offIcon.availableSizes())
        icon.addPixmap(offIcon.pixmap(s), QIcon::Normal, QIcon::Off);
    for (const QSize& s : onIcon.availableSizes())
        icon.addPixmap(onIcon.pixmap(s), QIcon::Normal, QIcon::On);
    return icon;
}

QString ClassBoardIcons::iconNameForFavorite(const QString& favorite)
{
    static const QHash<QString, QString> names = {
        {"openboardtool://ruler", "ruler"},
        {"openboardtool://triangle", "triangle"},
        {"openboardtool://protractor", "protractor"},
        {"openboardtool://compass", "compass"},
        {"openboardtool://axes", "axes"},
        {"openboardtool://openboard/mask", "mask"},
        {"openboardtool://cache", "spotlight"},
        {"openboardtool://magnifier", "magnifier"},
        {"widget:Minuteur", "timer"},
        {"widget:TirageAuSort", "dice"},
        {"widget:Vocabulaire", "vocabulaire"},
        {"widget:Etiquettes", "etiquettes"},
        {"widget:LecteurAudio", "audio"},
        {"widget:Stopwatch", "stopwatch"},
        {"widget:Calculator", "calculator"},
        {"widget:GraphMe", "graph"},
        {"widget:Horloge", "clock"},
    };

    return names.value(favorite);
}

void ClassBoardIcons::applyDrawingToolIcons(UBMainWindow* w, bool withMenuArrow)
{
    if (!w)
        return;

    const bool dark = ClassBoardTheme::isDark();
    if (withMenuArrow)
    {
        w->actionPen->setIcon(icon("pen", Style::Tool, true, dark ? QColor("#60A5FA") : QColor("#2563EB")));
        w->actionMarker->setIcon(icon("marker", Style::Tool, true, dark ? QColor("#FBBF24") : QColor("#D97706")));
        w->actionEraser->setIcon(icon("eraser", Style::Tool, true, dark ? QColor("#FB7185") : QColor("#E11D48")));
    }
    else
    {
        w->actionPen->setIcon(duotoneIcon("pen",
                                          dark ? QColor("#93C5FD") : QColor("#2563EB"),
                                          dark ? QColor(37, 99, 235, 55) : QColor("#DBEAFE"),
                                          QColor("#FFFFFF"), QColor("#2563EB")));
        w->actionMarker->setIcon(duotoneIcon("marker",
                                             dark ? QColor("#FCD34D") : QColor("#B45309"),
                                             dark ? QColor(217, 119, 6, 55) : QColor("#FEF3C7"),
                                             QColor("#78350F"), QColor("#FDE047")));
        w->actionEraser->setIcon(duotoneIcon("eraser",
                                             dark ? QColor("#FDA4AF") : QColor("#E11D48"),
                                             dark ? QColor(225, 29, 72, 55) : QColor("#FFE4E6"),
                                             QColor("#FFFFFF"), QColor("#E11D48")));
    }
}

void ClassBoardIcons::applyToMainWindow(UBMainWindow* w)
{
    if (!w)
        return;

    const bool dark = ClassBoardTheme::isDark();

    // Palette des stylets
    applyDrawingToolIcons(w, false);
    w->actionSelector->setIcon(duotoneIcon("selector",
                                           dark ? QColor("#A5B4FC") : QColor("#4F46E5"),
                                           dark ? QColor(79, 70, 229, 55) : QColor("#EEF2FF"),
                                           QColor("#FFFFFF"), QColor("#4F46E5")));
    w->actionPlay->setIcon(icon("play"));
    w->actionHand->setIcon(icon("hand"));
    w->actionZoomIn->setIcon(icon("zoom-in"));
    w->actionZoomOut->setIcon(icon("zoom-out"));
    w->actionPointer->setIcon(icon("laser"));
    w->actionLine->setIcon(icon("line"));
    w->actionText->setIcon(duotoneIcon("text",
                                       dark ? QColor("#CBD5E1") : QColor("#475569"),
                                       dark ? QColor(148, 163, 184, 45) : QColor("#F1F5F9"),
                                       QColor("#FFFFFF"), QColor("#475569")));
    w->actionCapture->setIcon(icon("capture"));
    w->actionVirtualKeyboard->setIcon(icon("keyboard"));
    w->actionSnap->setIcon(icon("snap"));

    // Libellés simples et directs affichés sous les icônes
    w->actionPen->setIconText(QObject::tr("Écrire"));
    w->actionEraser->setIconText(QObject::tr("Gommer"));
    w->actionMarker->setIconText(QObject::tr("Surligner"));
    w->actionSelector->setIconText(QObject::tr("Choisir"));
    w->actionPlay->setIconText(QObject::tr("Interagir"));
    w->actionHand->setIconText(QObject::tr("Déplacer"));
    w->actionZoomIn->setIconText(QObject::tr("Zoom +"));
    w->actionZoomOut->setIconText(QObject::tr("Zoom -"));
    w->actionPointer->setIconText(QObject::tr("Laser"));
    w->actionLine->setIconText(QObject::tr("Ligne"));
    w->actionText->setIconText(QObject::tr("Texte"));
    w->actionCapture->setIconText(QObject::tr("Capture"));
    w->actionVirtualKeyboard->setIconText(QObject::tr("Clavier"));
    w->actionSnap->setIconText(QObject::tr("Aimanter"));

    // Barre supérieure
    w->actionStylus->setIcon(icon("stylus", Style::Toolbar));
    w->actionBackgrounds->setIcon(icon("backgrounds", Style::Toolbar));
    w->actionUndo->setIcon(icon("undo", Style::Toolbar));
    w->actionRedo->setIcon(icon("redo", Style::Toolbar));
    w->actionPages->setIcon(icon("new-page", Style::Toolbar));
    w->actionBack->setIcon(icon("previous", Style::Toolbar));
    w->actionForward->setIcon(icon("next", Style::Toolbar));
    w->actionErase->setIcon(icon("clear-page", Style::Toolbar));
    w->actionBoard->setIcon(icon("board", Style::Toolbar));
    w->actionWeb->setIcon(icon("web", Style::Toolbar));
    w->actionDocument->setIcon(icon("documents", Style::Toolbar));
    w->actionDesktop->setIcon(icon("desktop", Style::Toolbar));
    w->actionMenu->setIcon(icon("menu", Style::Toolbar));

    // Mode Web
    w->actionWebBack->setIcon(icon("previous", Style::Toolbar));
    w->actionWebForward->setIcon(icon("next", Style::Toolbar));
    w->actionWebReload->setIcon(icon("reload", Style::Toolbar));
    w->actionStopLoading->setIcon(icon("stop", Style::Toolbar));
    w->actionHome->setIcon(icon("home", Style::Toolbar));
    w->actionWebBigger->setIcon(icon("zoom-in", Style::Toolbar));
    w->actionWebSmaller->setIcon(icon("zoom-out", Style::Toolbar));
    w->actionWebTools->setIcon(icon("tools", Style::Toolbar));
    w->actionBookmarks->setIcon(icon("bookmark", Style::Toolbar));
    w->actionAddBookmark->setIcon(icon("bookmark", Style::Toolbar));

    // Mode Documents
    w->actionNewFolder->setIcon(icon("new-folder", Style::Toolbar));
    w->actionNewDocument->setIcon(icon("new-page", Style::Toolbar));
    w->actionImport->setIcon(icon("import", Style::Toolbar));
    w->actionExport->setIcon(icon("export", Style::Toolbar));
    w->actionOpen->setIcon(icon("open", Style::Toolbar));
    w->actionDuplicate->setIcon(icon("duplicate", Style::Toolbar));
    w->actionDelete->setIcon(icon("clear-page", Style::Toolbar));
    w->actionRename->setIcon(icon("rename", Style::Toolbar));
    w->actionAddToWorkingDocument->setIcon(icon("add", Style::Toolbar));
    w->actionDocumentAdd->setIcon(icon("add", Style::Toolbar));
    w->actionDocumentTools->setIcon(icon("tools", Style::Toolbar));
    w->actionPreferences->setIcon(icon("menu", Style::Toolbar));
}
