/*
 * Copyright (C) 2026 Contributeurs ClassBoard
 *
 * This file is part of ClassBoard, a fork of OpenBoard. GPL-3.0.
 *
 * Tests unitaires du profil enseignant et du catalogue des disciplines.
 */

#include <QtTest>
#include <QTemporaryDir>

#include "classboard/ClassBoardDisciplines.h"
#include "classboard/ClassBoardProfile.h"

class TestClassBoardProfile : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir mDir;

private slots:
    void init()
    {
        QVERIFY(mDir.isValid());
        ClassBoardProfile::setProfileFileOverride(mDir.filePath("profile.ini"));
        ClassBoardProfile::reset();
    }

    void cleanupTestCase()
    {
        ClassBoardProfile::setProfileFileOverride(QString());
    }

    // --- Catalogue -------------------------------------------------------

    void catalogContainsAllRequiredDisciplines()
    {
        const QStringList expected = {"general", "math", "francais", "histoire-geo", "anglais",
                                      "langues", "physique-chimie", "svt", "technologie",
                                      "arts-plastiques", "musique", "eps", "segpa", "ulis", "autre"};

        QCOMPARE(ClassBoardDisciplines::all().size(), expected.size());

        for (const QString& id : expected)
        {
            const ClassBoardDiscipline d = ClassBoardDisciplines::byId(id);
            QVERIFY2(d.isValid(), qPrintable(id));
            QVERIFY2(!d.displayName().isEmpty(), qPrintable(id));
            QVERIFY2(!d.favorites.isEmpty(), qPrintable(id));
        }
    }

    void catalogIdsAreUnique()
    {
        QSet<QString> ids;
        for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
        {
            QVERIFY2(!ids.contains(d.id), qPrintable(d.id));
            ids.insert(d.id);
        }
    }

    void validIdsFiltersUnknownAndDuplicates()
    {
        const QStringList ids = ClassBoardDisciplines::validIds({"math", "inconnu", " math ", "svt", ""});
        QCOMPARE(ids, QStringList({"math", "svt"}));
    }

    void favoritesDefaultToGeneral()
    {
        QCOMPARE(ClassBoardDisciplines::favoritesFor({}), ClassBoardDisciplines::byId("general").favorites);
    }

    void favoritesOfSingleDisciplineKeepOrder()
    {
        const QStringList favorites = ClassBoardDisciplines::favoritesFor({"math"});
        QCOMPARE(favorites.first(), QString("openboardtool://ruler"));
        QCOMPARE(favorites, ClassBoardDisciplines::byId("math").favorites);
    }

    void favoritesAreMergedFairlyWithoutDuplicates()
    {
        const QStringList favorites = ClassBoardDisciplines::favoritesFor({"math", "francais"}, 4);

        // Chaque discipline place son outil principal en premier
        QCOMPARE(favorites.size(), 4);
        QCOMPARE(favorites.at(0), QString("openboardtool://ruler"));
        QCOMPARE(favorites.at(1), QString("openboardtool://openboard/mask"));
        QCOMPARE(QSet<QString>(favorites.begin(), favorites.end()).size(), favorites.size());
    }

    void favoritesRespectMaximum()
    {
        QStringList all;
        for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
            all << d.id;

        QVERIFY(ClassBoardDisciplines::favoritesFor(all, 9).size() <= 9);
        QVERIFY(ClassBoardDisciplines::favoritesFor(all, 3).size() == 3);
    }

    void seyesSuggestion()
    {
        QVERIFY(ClassBoardDisciplines::suggestsSeyes({"francais"}));
        QVERIFY(ClassBoardDisciplines::suggestsSeyes({"math", "ulis"}));
        QVERIFY(!ClassBoardDisciplines::suggestsSeyes({"math", "eps"}));
    }

    void favoriteNames()
    {
        QCOMPARE(ClassBoardDisciplines::favoriteDisplayName("openboardtool://compass"), QString("Compas"));
        QCOMPARE(ClassBoardDisciplines::favoriteDisplayName("widget:Minuteur"), QString("Minuteur"));
        QCOMPARE(ClassBoardDisciplines::favoriteDisplayName("widget:MonOutil"), QString("MonOutil"));
    }

    // --- Profil ----------------------------------------------------------

    void defaultsOnFirstLaunch()
    {
        QVERIFY(!ClassBoardProfile::isFirstRunCompleted());
        QVERIFY(ClassBoardProfile::disciplines().isEmpty());
        QCOMPARE(ClassBoardProfile::theme(), ClassBoardProfile::Theme::Light);
        QCOMPARE(ClassBoardProfile::toolbarScale(), 125);
        QVERIFY(ClassBoardProfile::showToolLabels());
        QVERIFY(ClassBoardProfile::showFavoritesBar());
        QVERIFY(ClassBoardProfile::interfaceLanguage().isEmpty());
    }

    void disciplinesArePersisted()
    {
        ClassBoardProfile::setDisciplines({"math", "physique-chimie", "nimporte-quoi"});
        ClassBoardProfile::setFirstRunCompleted(true);

        QCOMPARE(ClassBoardProfile::disciplines(), QStringList({"math", "physique-chimie"}));
        QVERIFY(ClassBoardProfile::isFirstRunCompleted());

        QVERIFY(QFile::exists(ClassBoardProfile::profileFile()));
    }

    void themeRoundTrip()
    {
        for (auto theme : {ClassBoardProfile::Theme::Light, ClassBoardProfile::Theme::Dark, ClassBoardProfile::Theme::System})
        {
            ClassBoardProfile::setTheme(theme);
            QCOMPARE(ClassBoardProfile::theme(), theme);
        }

        QCOMPARE(ClassBoardProfile::themeFromString("DARK"), ClassBoardProfile::Theme::Dark);
        QCOMPARE(ClassBoardProfile::themeFromString("???"), ClassBoardProfile::Theme::Light);
    }

    void toolbarScaleIsBounded()
    {
        ClassBoardProfile::setToolbarScale(300);
        QCOMPARE(ClassBoardProfile::toolbarScale(), 150);
        ClassBoardProfile::setToolbarScale(10);
        QCOMPARE(ClassBoardProfile::toolbarScale(), 100);
    }

    void effectiveFavoritesFollowDisciplinesUntilCustomized()
    {
        ClassBoardProfile::setDisciplines({"eps"});
        QCOMPARE(ClassBoardProfile::effectiveFavorites(), ClassBoardDisciplines::favoritesFor({"eps"}));

        ClassBoardProfile::setCustomFavorites({"openboardtool://compass"});
        QCOMPARE(ClassBoardProfile::effectiveFavorites(), QStringList({"openboardtool://compass"}));

        ClassBoardProfile::setCustomFavorites({});
        QCOMPARE(ClassBoardProfile::effectiveFavorites(), ClassBoardDisciplines::favoritesFor({"eps"}));
    }

    void resetRestoresDefaults()
    {
        ClassBoardProfile::setDisciplines({"svt"});
        ClassBoardProfile::setFirstRunCompleted(true);
        ClassBoardProfile::setTheme(ClassBoardProfile::Theme::Dark);

        ClassBoardProfile::reset();

        QVERIFY(!ClassBoardProfile::isFirstRunCompleted());
        QVERIFY(ClassBoardProfile::disciplines().isEmpty());
        QCOMPARE(ClassBoardProfile::theme(), ClassBoardProfile::Theme::Light);
    }

    void portableModeUsesApplicationFolder()
    {
        qputenv("CLASSBOARD_PORTABLE", "1");
        QVERIFY(ClassBoardProfile::isPortable());
        QVERIFY(ClassBoardProfile::profileDirectory().startsWith(QCoreApplication::applicationDirPath()));
        qunsetenv("CLASSBOARD_PORTABLE");
    }

    void toolbarSideDefaultsRightAndRoundTrips()
    {
        QCOMPARE(ClassBoardProfile::toolbarSide(), ClassBoardProfile::ToolbarSide::Right);
        ClassBoardProfile::setToolbarSide(ClassBoardProfile::ToolbarSide::Left);
        QCOMPARE(ClassBoardProfile::toolbarSide(), ClassBoardProfile::ToolbarSide::Left);
        ClassBoardProfile::setToolbarSide(ClassBoardProfile::ToolbarSide::Right);
        QCOMPARE(ClassBoardProfile::toolbarSide(), ClassBoardProfile::ToolbarSide::Right);
        QCOMPARE(ClassBoardProfile::toolbarSideFromString("LEFT"), ClassBoardProfile::ToolbarSide::Left);
        QCOMPARE(ClassBoardProfile::toolbarSideFromString("other"), ClassBoardProfile::ToolbarSide::Right);
    }

    void languageWidgetsRegistered()
    {
        QCOMPARE(ClassBoardDisciplines::favoriteDisplayName("widget:Vocabulaire"), QString("Vocabulaire"));
        QCOMPARE(ClassBoardDisciplines::favoriteDisplayName("widget:Etiquettes"), QString("Étiquettes"));
        QVERIFY(ClassBoardDisciplines::byId("anglais").favorites.contains("widget:Vocabulaire"));
        QVERIFY(ClassBoardDisciplines::byId("francais").favorites.contains("widget:Etiquettes"));
    }
};

QTEST_GUILESS_MAIN(TestClassBoardProfile)
#include "tst_classboardprofile.moc"
