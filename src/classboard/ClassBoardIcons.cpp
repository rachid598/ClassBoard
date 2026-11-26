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

    w->actionPen->setIcon(icon("pen", Style::Tool, withMenuArrow));
    w->actionMarker->setIcon(icon("marker", Style::Tool, withMenuArrow));
    w->actionEraser->setIcon(icon("eraser", Style::Tool, withMenuArrow));
}

void ClassBoardIcons::applyToMainWindow(UBMainWindow* w)
{
    if (!w)
        return;

    // Palette des stylets
    applyDrawingToolIcons(w, false);
    w->actionSelector->setIcon(icon("selector"));
    w->actionPlay->setIcon(icon("play"));
    w->actionHand->setIcon(icon("hand"));
    w->actionZoomIn->setIcon(icon("zoom-in"));
    w->actionZoomOut->setIcon(icon("zoom-out"));
    w->actionPointer->setIcon(icon("laser"));
    w->actionLine->setIcon(icon("line"));
    w->actionText->setIcon(icon("text"));
    w->actionCapture->setIcon(icon("capture"));
    w->actionVirtualKeyboard->setIcon(icon("keyboard"));
    w->actionSnap->setIcon(icon("snap"));

    // Libellés courts affichés sous les icônes (les infobulles gardent le texte complet)
    w->actionPen->setIconText(QObject::tr("Stylo"));
    w->actionEraser->setIconText(QObject::tr("Gomme"));
    w->actionMarker->setIconText(QObject::tr("Surligneur"));
    w->actionSelector->setIconText(QObject::tr("Sélection"));
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
