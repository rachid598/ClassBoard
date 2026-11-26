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

#include "ClassBoardDisciplines.h"

#include <QCoreApplication>
#include <QSet>

const QString ClassBoardDisciplines::WidgetPrefix = QStringLiteral("widget:");

namespace
{
    // Instruments natifs du moteur OpenBoard
    const QString Ruler      = QStringLiteral("openboardtool://ruler");
    const QString Triangle   = QStringLiteral("openboardtool://triangle");
    const QString Protractor = QStringLiteral("openboardtool://protractor");
    const QString Compass    = QStringLiteral("openboardtool://compass");
    const QString Axes       = QStringLiteral("openboardtool://axes");
    const QString Mask       = QStringLiteral("openboardtool://openboard/mask");
    const QString Spotlight  = QStringLiteral("openboardtool://cache");
    const QString Magnifier  = QStringLiteral("openboardtool://magnifier");

    // Applications de la bibliothèque (fonctionnent hors connexion)
    const QString Timer       = QStringLiteral("widget:Minuteur");
    const QString Draw        = QStringLiteral("widget:TirageAuSort");
    const QString Vocabulaire = QStringLiteral("widget:Vocabulaire");
    const QString Etiquettes  = QStringLiteral("widget:Etiquettes");
    const QString Stopwatch   = QStringLiteral("widget:Stopwatch");
    const QString Calculator  = QStringLiteral("widget:Calculator");
    const QString Graph       = QStringLiteral("widget:GraphMe");
    const QString Clock       = QStringLiteral("widget:Horloge");

    QString icon(const char* name)
    {
        return QStringLiteral(":/classboard/disciplines/%1.svg").arg(QLatin1String(name));
    }

    QList<ClassBoardDiscipline> buildCatalog()
    {
        QList<ClassBoardDiscipline> list;

        auto add = [&list](const char* id, const char* name, const QStringList& favorites, bool seyes = false)
        {
            ClassBoardDiscipline d;
            d.id = QLatin1String(id);
            d.name = name;
            d.icon = icon(id);
            d.favorites = favorites;
            d.suggestsSeyes = seyes;
            list << d;
        };

        add("general", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Général / polyvalent"),
            {Timer, Draw, Mask, Spotlight, Vocabulaire, Etiquettes, Ruler, Magnifier});
        add("math", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Mathématiques"),
            {Ruler, Triangle, Protractor, Compass, Axes, Graph, Calculator});
        add("francais", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Français"),
            {Mask, Etiquettes, Timer, Draw, Vocabulaire, Spotlight, Magnifier}, true);
        add("histoire-geo", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Histoire-géographie / EMC"),
            {Mask, Spotlight, Magnifier, Timer, Draw, Ruler});
        add("anglais", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Anglais"),
            {Vocabulaire, Timer, Draw, Mask, Etiquettes, Spotlight});
        add("langues", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Autres langues vivantes"),
            {Vocabulaire, Timer, Draw, Mask, Etiquettes, Spotlight});
        add("physique-chimie", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Physique-chimie"),
            {Ruler, Protractor, Axes, Graph, Calculator, Stopwatch});
        add("svt", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "SVT"),
            {Magnifier, Mask, Spotlight, Ruler, Timer});
        add("technologie", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Technologie"),
            {Ruler, Triangle, Compass, Protractor, Axes, Calculator});
        add("arts-plastiques", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Arts plastiques"),
            {Magnifier, Mask, Spotlight, Ruler, Compass, Timer});
        add("musique", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Éducation musicale"),
            {Timer, Stopwatch, Clock, Mask, Draw});
        add("eps", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "EPS"),
            {Stopwatch, Timer, Draw, Clock});
        add("segpa", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "SEGPA"),
            {Timer, Draw, Etiquettes, Vocabulaire, Mask, Spotlight, Magnifier, Ruler}, true);
        add("ulis", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "ULIS"),
            {Timer, Etiquettes, Vocabulaire, Mask, Spotlight, Magnifier, Draw}, true);
        add("autre", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Autres disciplines"),
            {Timer, Draw, Mask, Spotlight, Ruler});

        return list;
    }
}

QString ClassBoardDiscipline::displayName() const
{
    return name ? QCoreApplication::translate("ClassBoardDisciplines", name) : QString();
}

const QList<ClassBoardDiscipline>& ClassBoardDisciplines::all()
{
    static const QList<ClassBoardDiscipline> catalog = buildCatalog();
    return catalog;
}

ClassBoardDiscipline ClassBoardDisciplines::byId(const QString& id)
{
    for (const ClassBoardDiscipline& d : all())
    {
        if (d.id == id)
            return d;
    }

    return ClassBoardDiscipline();
}

QStringList ClassBoardDisciplines::validIds(const QStringList& ids)
{
    QStringList result;

    for (const QString& raw : ids)
    {
        const QString id = raw.trimmed();

        if (!result.contains(id) && byId(id).isValid())
            result << id;
    }

    return result;
}

QStringList ClassBoardDisciplines::favoritesFor(const QStringList& disciplineIds, int maximum)
{
    QStringList ids = validIds(disciplineIds);

    if (ids.isEmpty())
        ids << QStringLiteral("general");

    QList<QStringList> lists;
    for (const QString& id : ids)
        lists << byId(id).favorites;

    // Fusion "à tour de rôle" : chaque discipline place d'abord son outil
    // principal, puis le suivant, etc. Aucune discipline n'écrase les autres.
    QStringList result;
    bool added = true;

    for (int rank = 0; added && result.size() < maximum; ++rank)
    {
        added = false;

        for (const QStringList& favorites : lists)
        {
            if (rank < favorites.size())
            {
                added = true;
                const QString& favorite = favorites.at(rank);

                if (!result.contains(favorite))
                    result << favorite;

                if (result.size() >= maximum)
                    break;
            }
        }
    }

    return result;
}

bool ClassBoardDisciplines::suggestsSeyes(const QStringList& disciplineIds)
{
    for (const QString& id : disciplineIds)
    {
        if (byId(id).suggestsSeyes)
            return true;
    }

    return false;
}

QString ClassBoardDisciplines::favoriteDisplayName(const QString& favorite)
{
    struct Name { const char* key; const char* name; };

    static const Name names[] = {
        {"openboardtool://ruler",          QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Règle")},
        {"openboardtool://triangle",       QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Équerre")},
        {"openboardtool://protractor",     QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Rapporteur")},
        {"openboardtool://compass",        QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Compas")},
        {"openboardtool://axes",           QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Repère")},
        {"openboardtool://openboard/mask", QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Masque")},
        {"openboardtool://cache",          QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Projecteur")},
        {"openboardtool://magnifier",      QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Loupe")},
        {"widget:Minuteur",                QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Minuteur")},
        {"widget:TirageAuSort",            QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Tirage au sort")},
        {"widget:Vocabulaire",             QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Vocabulaire")},
        {"widget:Etiquettes",              QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Étiquettes")},
        {"widget:Stopwatch",               QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Chronomètre")},
        {"widget:Calculator",              QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Calculatrice")},
        {"widget:GraphMe",                 QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Graphique")},
        {"widget:Horloge",                 QT_TRANSLATE_NOOP("ClassBoardDisciplines", "Horloge")},
    };

    for (const Name& n : names)
    {
        if (favorite == QLatin1String(n.key))
            return QCoreApplication::translate("ClassBoardDisciplines", n.name);
    }

    if (favorite.startsWith(WidgetPrefix))
        return favorite.mid(WidgetPrefix.size());

    return favorite;
}
