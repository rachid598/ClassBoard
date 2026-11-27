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

#include <QColor>
#include <QFrame>
#include <QList>
#include <QString>

class QButtonGroup;
class QHBoxLayout;
class QLineEdit;
class QVBoxLayout;
class QWidget;

/**
 * @brief Tiroir flottant « Outils pédagogiques » filtré par discipline.
 *
 * S'ouvre juste à côté du bouton « Outils » sur la barre verticale du tableau.
 * Met en avant les outils adaptés aux matières choisies par l'enseignant
 * (ex. Anglais, Français, Classe) tout en gardant tous les autres accessibles.
 */
class ClassBoardToolsDrawer : public QFrame
{
    Q_OBJECT

public:
    explicit ClassBoardToolsDrawer(QWidget* parent = nullptr);

    void applyTheme();
    void reloadForProfile();

signals:
    void toolLaunched();
    void closed();

private:
    enum class ActionKind
    {
        NativeTool,
        Widget,
        ShapeSvg,
        Capture,
        ImportImage,
        ImportPdf,
        ExportPdf,
        LineTool,
        LaserTool
    };

    struct ToolEntry
    {
        QString id;
        QString label;
        QString keywords;
        QString section;
        QString iconName;
        QColor fgColor;
        QColor bgColor;
        ActionKind kind = ActionKind::NativeTool;
        QString target;
        QStringList tags; // "anglais", "francais", "classe", "math", "all"
    };

    void buildCatalog();
    void rebuildFilterChips();
    void rebuildGrid();
    void launchEntry(const ToolEntry& entry);

    QLineEdit* mSearchEdit = nullptr;
    QWidget* mChipHost = nullptr;
    QHBoxLayout* mChipLayout = nullptr;
    QButtonGroup* mChipGroup = nullptr;
    QWidget* mGridHost = nullptr;
    QVBoxLayout* mGridLayout = nullptr;

    QList<ToolEntry> mCatalog;
    QString mActiveFilter;
};
