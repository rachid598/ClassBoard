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

#include "ClassBoardFavoritesPalette.h"

#include "ClassBoardDisciplines.h"
#include "ClassBoardIcons.h"
#include "ClassBoardProfile.h"
#include "ClassBoardTheme.h"

#include "board/UBBoardController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "tools/UBToolsManager.h"

#include <QFile>
#include <QLayout>

ClassBoardFavoritesPalette::ClassBoardFavoritesPalette(QWidget* parent)
    : UBActionPalette(Qt::TopLeftCorner, parent, Qt::Vertical)
{
    setObjectName("ClassBoardFavoritesPalette");
    setButtonIconSize(ClassBoardTheme::scaledIconSize(36));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    reload();
}

QIcon ClassBoardFavoritesPalette::iconForFavorite(const QString& favorite, QString* label, QUrl* url)
{
    *label = ClassBoardDisciplines::favoriteDisplayName(favorite);

    if (favorite.startsWith(ClassBoardDisciplines::WidgetPrefix))
    {
        const QString name = favorite.mid(ClassBoardDisciplines::WidgetPrefix.size());
        const QString path = UBSettings::settings()->applicationApplicationsLibraryDirectory() + "/" + name + ".wgt";

        if (!QFile::exists(path))
            return QIcon();

        *url = QUrl::fromLocalFile(path);

        const QString iconName = ClassBoardIcons::iconNameForFavorite(favorite);
        if (!iconName.isEmpty())
            return ClassBoardIcons::icon(iconName, ClassBoardIcons::Style::Favorite);

        return QIcon(UBGraphicsWidgetItem::iconFilePath(*url));
    }

    const UBToolsManager::UBToolDescriptor desc = UBToolsManager::manager()->toolByID(favorite);

    if (desc.id.isEmpty())
        return QIcon();

    *url = QUrl(desc.id);

    const QString iconName = ClassBoardIcons::iconNameForFavorite(favorite);
    if (!iconName.isEmpty())
        return ClassBoardIcons::icon(iconName, ClassBoardIcons::Style::Favorite);

    return QIcon(desc.icon);
}

void ClassBoardFavoritesPalette::reload()
{
    for (QAction* action : mActions)
        action->deleteLater();

    clearLayout();
    mActions.clear();
    mButtons.clear();
    mMapActionToButton.clear();

    const QStringList favorites = ClassBoardProfile::effectiveFavorites();

    for (const QString& favorite : favorites)
    {
        QString label;
        QUrl url;
        const QIcon icon = iconForFavorite(favorite, &label, &url);

        if (icon.isNull() || url.isEmpty())
            continue;

        QAction* action = new QAction(icon, label, this);
        action->setToolTip(label);
        action->setData(url);
        connect(action, &QAction::triggered, this, &ClassBoardFavoritesPalette::favoriteTriggered);
        addAction(action);
    }

    QAction* edit = new QAction(ClassBoardTheme::tintedIcon(":/classboard/icons/star-plus.svg", ClassBoardTheme::colors().textSecondary, 36),
                                tr("Modifier mes favoris"), this);
    edit->setToolTip(tr("Modifier mes favoris"));
    connect(edit, &QAction::triggered, this, &ClassBoardFavoritesPalette::editRequested);
    addAction(edit);

    // Les boutons ajoutés à une barre déjà visible ne s'affichent qu'au prochain
    // tour de boucle : on les montre tout de suite pour calculer la bonne taille.
    for (UBActionPaletteButton* button : mButtons)
        button->show();

    if (layout())
        layout()->activate();

    adjustSize();
}

void ClassBoardFavoritesPalette::placeInContainer(const QRect& area)
{
    // Petit écran : on réduit les boutons pour que la barre tienne dans la zone libre
    QSize iconSize = ClassBoardTheme::scaledIconSize(36);
    setButtonIconSize(iconSize);
    adjustSize();

    while (height() > area.height() && iconSize.height() > 24)
    {
        iconSize -= QSize(4, 4);
        setButtonIconSize(iconSize);
        adjustSize();
    }

    const int x = area.left() + 34;   // laisse visible l'onglet du panneau des pages
    const int y = area.top() + qMax(0, (area.height() - height()) / 2);
    move(x, y);
}

void ClassBoardFavoritesPalette::favoriteTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (action && UBApplication::boardController)
        UBApplication::boardController->downloadURL(action->data().toUrl());
}
