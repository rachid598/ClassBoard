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

#include "ClassBoardSetupDialog.h"

#include "ClassBoardDisciplines.h"
#include "ClassBoardProfile.h"
#include "ClassBoardTheme.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    enum Page { DisciplinesPage = 0, AppearancePage = 1 };

    struct PalettePair { QColor fg; QColor bg; };

    PalettePair disciplinePalette(const QString& id)
    {
        if (id == "anglais")         return {QColor("#4F46E5"), QColor("#EEF2FF")};
        if (id == "francais")        return {QColor("#2563EB"), QColor("#DBEAFE")};
        if (id == "histoire-geo")    return {QColor("#EA580C"), QColor("#FFEDD5")};
        if (id == "langues")         return {QColor("#D97706"), QColor("#FEF3C7")};
        if (id == "svt")             return {QColor("#059669"), QColor("#D1FAE5")};
        if (id == "physique-chimie") return {QColor("#DB2777"), QColor("#FCE7F3")};
        if (id == "math")            return {QColor("#9333EA"), QColor("#F3E8FF")};
        if (id == "technologie")     return {QColor("#0284C7"), QColor("#E0F2FE")};
        if (id == "arts-plastiques") return {QColor("#E11D48"), QColor("#FFE4E6")};
        if (id == "musique")         return {QColor("#7C3AED"), QColor("#EDE9FE")};
        if (id == "eps")             return {QColor("#16A34A"), QColor("#DCFCE7")};
        if (id == "general")         return {QColor("#D97706"), QColor("#FEF3C7")};
        if (id == "segpa")           return {QColor("#0D9488"), QColor("#CCFBF1")};
        if (id == "ulis")            return {QColor("#7C3AED"), QColor("#F3E8FF")};
        return {QColor("#475569"), QColor("#F1F5F9")};
    }

    QString shortDisciplineName(const ClassBoardDiscipline& d)
    {
        if (d.id == "histoire-geo")    return QObject::tr("Histoire-Géo");
        if (d.id == "langues")         return QObject::tr("Autres langues");
        if (d.id == "general")         return QObject::tr("Polyvalent");
        if (d.id == "arts-plastiques") return QObject::tr("Arts plastiques");
        if (d.id == "musique")         return QObject::tr("Musique");
        if (d.id == "autre")           return QObject::tr("Autres");
        return d.displayName();
    }

    // Ordre d'affichage : langues & lettres mises en avant sans exclure aucune discipline
    QList<ClassBoardDiscipline> orderedDisciplines()
    {
        static const QStringList preferred = {
            "anglais", "francais", "histoire-geo", "langues", "svt",
            "physique-chimie", "math", "technologie", "arts-plastiques", "musique",
            "eps", "general", "segpa", "ulis", "autre"
        };

        QList<ClassBoardDiscipline> result;
        for (const QString& id : preferred)
        {
            const ClassBoardDiscipline d = ClassBoardDisciplines::byId(id);
            if (d.isValid())
                result << d;
        }
        for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
        {
            if (!preferred.contains(d.id))
                result << d;
        }
        return result;
    }

    QIcon disciplineBadgeIcon(const ClassBoardDiscipline& d)
    {
        const PalettePair pal = disciplinePalette(d.id);
        const bool dark = ClassBoardTheme::isDark();
        const QColor bgOff = dark ? QColor(pal.fg.red(), pal.fg.green(), pal.fg.blue(), 48) : pal.bg;
        const QColor fgOff = dark ? pal.fg.lighter(135) : pal.fg;

        const QIcon off = ClassBoardTheme::duotoneIcon(d.icon, fgOff, bgOff, 46, 14);
        const QIcon on = ClassBoardTheme::duotoneIcon(d.icon, QColor("#FFFFFF"), pal.fg, 46, 14);

        QIcon icon;
        for (const QSize& size : off.availableSizes())
            icon.addPixmap(off.pixmap(size), QIcon::Normal, QIcon::Off);
        for (const QSize& size : on.availableSizes())
            icon.addPixmap(on.pixmap(size), QIcon::Normal, QIcon::On);
        return icon;
    }
}

ClassBoardSetupDialog::ClassBoardSetupDialog(Mode mode, QWidget* parent)
    : QDialog(parent)
    , mMode(mode)
{
    setObjectName("ClassBoardSetupDialog");
    setWindowTitle(mode == Mode::FirstRun ? tr("Bienvenue sur ClassBoard") : tr("Mes matières et préférences"));
    setWindowIcon(QIcon(":/images/ClassBoard.png"));
    setStyleSheet(ClassBoardTheme::dialogStyleSheet());
    setModal(true);
    resize(1020, 720);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 24, 28, 24);

    QWidget* card = new QWidget(this);
    card->setObjectName("cbCard");
    card->setAttribute(Qt::WA_StyledBackground, true);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(36);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(15, 23, 42, 32));
    card->setGraphicsEffect(shadow);
    outer->addWidget(card);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(38, 30, 38, 26);
    layout->setSpacing(20);

    layout->addWidget(buildHeader());

    mPages = new QStackedWidget(card);
    mPages->addWidget(buildDisciplinesPage());
    mPages->addWidget(buildAppearancePage());
    layout->addWidget(mPages, 1);

    layout->addWidget(buildFooter());

    loadFromProfile();
    updateState();
}

bool ClassBoardSetupDialog::runFirstLaunchIfNeeded()
{
    if (ClassBoardProfile::isFirstRunCompleted())
        return false;

    if (qEnvironmentVariableIntValue("CLASSBOARD_SKIP_FIRST_RUN") == 1)
        return false;

    ClassBoardSetupDialog dialog(Mode::FirstRun);
    dialog.exec();

    ClassBoardProfile::setFirstRunCompleted(true);
    return true;
}

QWidget* ClassBoardSetupDialog::buildHeader()
{
    QWidget* header = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(header);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(12);

    if (mMode == Mode::FirstRun)
        row->addStretch(1);

    QLabel* logo = new QLabel(header);
    logo->setPixmap(QIcon(":/images/ClassBoard.png").pixmap(42, 42));
    row->addWidget(logo);

    QLabel* name = new QLabel(mMode == Mode::FirstRun ? tr("Bienvenue sur ClassBoard")
                                                      : tr("Mes matières et préférences"), header);
    name->setStyleSheet("font-size: 18px; font-weight: 800;");
    row->addWidget(name);
    row->addStretch(1);

    if (mMode == Mode::Settings)
    {
        mNavGroup = new QButtonGroup(this);
        mNavGroup->setExclusive(true);
        const QStringList titles = {tr("Mes matières"), tr("Apparence & barre")};

        for (int i = 0; i < titles.size(); ++i)
        {
            QPushButton* button = new QPushButton(titles.at(i), header);
            button->setCheckable(true);
            button->setObjectName("cbNav");
            button->setCursor(Qt::PointingHandCursor);
            mNavGroup->addButton(button, i);
            row->addWidget(button);
        }

        mNavGroup->button(0)->setChecked(true);
        connect(mNavGroup, &QButtonGroup::idClicked, this, [this](int id) {
            mPages->setCurrentIndex(id);
        });
    }

    return header;
}

QWidget* ClassBoardSetupDialog::buildDisciplinesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* title = new QLabel(tr("Quelle matière enseignez-vous ?"), page);
    title->setObjectName("cbTitle");
    if (mMode == Mode::FirstRun)
        title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("ClassBoard prépare vos outils essentiels dès le démarrage. "
                                     "Tout reste accessible en un clic."), page);
    subtitle->setObjectName("cbSubtitle");
    subtitle->setWordWrap(true);
    if (mMode == Mode::FirstRun)
        subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);
    layout->addSpacing(10);

    QGridLayout* grid = new QGridLayout();
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    const QList<ClassBoardDiscipline> list = orderedDisciplines();
    for (int i = 0; i < list.size(); ++i)
    {
        const ClassBoardDiscipline& d = list.at(i);
        QToolButton* button = new QToolButton(page);
        button->setObjectName("cbDiscipline");
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIcon(disciplineBadgeIcon(d));
        button->setIconSize(QSize(46, 46));
        button->setText(shortDisciplineName(d));
        button->setToolTip(d.displayName());
        button->setAccessibleName(d.displayName());
        button->setMinimumSize(156, 108);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QToolButton::toggled, this, &ClassBoardSetupDialog::updateState);

        grid->addWidget(button, i / 5, i % 5);
        mDisciplineButtons.insert(d.id, button);
    }

    layout->addLayout(grid, 1);

    mSelectionSummary = new QLabel(page);
    mSelectionSummary->setObjectName("cbHint");
    if (mMode == Mode::FirstRun)
        mSelectionSummary->setAlignment(Qt::AlignCenter);
    layout->addWidget(mSelectionSummary);

    return page;
}

QWidget* ClassBoardSetupDialog::buildAppearancePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QLabel* title = new QLabel(tr("Apparence et confort au tableau"), page);
    title->setObjectName("cbTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Tous ces réglages s'appliquent immédiatement sans redémarrer."), page);
    subtitle->setObjectName("cbSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    layout->addSpacing(12);

    QHBoxLayout* columns = new QHBoxLayout();
    columns->setSpacing(48);

    QVBoxLayout* leftCol = new QVBoxLayout();
    leftCol->setSpacing(6);

    QLabel* sideTitle = new QLabel(tr("Position de la barre d'outils"), page);
    sideTitle->setObjectName("cbSection");
    leftCol->addWidget(sideTitle);

    mSideRight = new QRadioButton(tr("À droite (laisse le bas libre)"), page);
    mSideLeft  = new QRadioButton(tr("À gauche (confort gaucher)"), page);
    QButtonGroup* sideGroup = new QButtonGroup(page);
    sideGroup->addButton(mSideRight);
    sideGroup->addButton(mSideLeft);
    leftCol->addWidget(mSideRight);
    leftCol->addWidget(mSideLeft);
    leftCol->addSpacing(16);

    QLabel* themeTitle = new QLabel(tr("Couleur de l'interface"), page);
    themeTitle->setObjectName("cbSection");
    leftCol->addWidget(themeTitle);

    mThemeLight  = new QRadioButton(tr("Clair — tableau blanc (par défaut)"), page);
    mThemeDark   = new QRadioButton(tr("Sombre — mode ardoise anti-éblouissement"), page);
    mThemeSystem = new QRadioButton(tr("Identique à Windows"), page);
    QButtonGroup* themeGroup = new QButtonGroup(page);
    themeGroup->addButton(mThemeLight);
    themeGroup->addButton(mThemeDark);
    themeGroup->addButton(mThemeSystem);
    leftCol->addWidget(mThemeLight);
    leftCol->addWidget(mThemeDark);
    leftCol->addWidget(mThemeSystem);
    leftCol->addStretch(1);
    columns->addLayout(leftCol);

    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setSpacing(6);

    QLabel* sizeTitle = new QLabel(tr("Taille des boutons"), page);
    sizeTitle->setObjectName("cbSection");
    rightCol->addWidget(sizeTitle);

    mSize100 = new QRadioButton(tr("Standard"), page);
    mSize125 = new QRadioButton(tr("Grande (recommandée sur TBI)"), page);
    mSize150 = new QRadioButton(tr("Très grande"), page);
    QButtonGroup* sizeGroup = new QButtonGroup(page);
    sizeGroup->addButton(mSize100);
    sizeGroup->addButton(mSize125);
    sizeGroup->addButton(mSize150);
    rightCol->addWidget(mSize100);
    rightCol->addWidget(mSize125);
    rightCol->addWidget(mSize150);
    rightCol->addSpacing(16);

    QLabel* optTitle = new QLabel(tr("Lisibilité"), page);
    optTitle->setObjectName("cbSection");
    rightCol->addWidget(optTitle);

    mShowLabels = new QCheckBox(tr("Afficher le nom des outils sous les icônes"), page);
    rightCol->addWidget(mShowLabels);
    rightCol->addStretch(1);
    columns->addLayout(rightCol);

    layout->addLayout(columns, 1);

    // Dossier de sauvegarde des cours (ex. lecteur réseau Kwartz)
    QLabel* storageTitle = new QLabel(tr("Dossier de sauvegarde des cours (lecteur réseau Kwartz, clé USB…)"), page);
    storageTitle->setObjectName("cbSection");
    layout->addWidget(storageTitle);

    QHBoxLayout* storageRow = new QHBoxLayout();
    storageRow->setSpacing(10);
    mDocsDirLabel = new QLabel(page);
    mDocsDirLabel->setObjectName("cbHint");
    mDocsDirLabel->setWordWrap(true);
    storageRow->addWidget(mDocsDirLabel, 1);

    QPushButton* chooseDirBtn = new QPushButton(tr("Choisir un dossier…"), page);
    chooseDirBtn->setCursor(Qt::PointingHandCursor);
    connect(chooseDirBtn, &QPushButton::clicked, this, [this]() {
        const QString initial = mPendingDocsDir.isEmpty() ? QDir::homePath() : mPendingDocsDir;
        const QString picked = QFileDialog::getExistingDirectory(this, tr("Choisir le dossier des cours ClassBoard"), initial);
        if (!picked.isEmpty())
        {
            mPendingDocsDir = picked;
            mDocsDirLabel->setText(tr("%1 (actif au prochain démarrage)").arg(QDir::toNativeSeparators(picked)));
        }
    });
    storageRow->addWidget(chooseDirBtn);

    QPushButton* defaultDirBtn = new QPushButton(tr("Par défaut"), page);
    defaultDirBtn->setCursor(Qt::PointingHandCursor);
    connect(defaultDirBtn, &QPushButton::clicked, this, [this]() {
        mPendingDocsDir.clear();
        mDocsDirLabel->setText(tr("Dossier local par défaut (actif au prochain démarrage)"));
    });
    storageRow->addWidget(defaultDirBtn);
    layout->addLayout(storageRow);

    QLabel* where = new QLabel(tr("Profil enregistré : %1")
                                   .arg(QDir::toNativeSeparators(ClassBoardProfile::profileFile())), page);
    where->setObjectName("cbHint");
    where->setWordWrap(true);
    layout->addWidget(where);

    return page;
}

QWidget* ClassBoardSetupDialog::buildFooter()
{
    QWidget* footer = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(footer);
    row->setContentsMargins(0, 4, 0, 0);
    row->setSpacing(12);

    if (mMode == Mode::FirstRun)
    {
        QPushButton* skipBtn = new QPushButton(tr("Passer"), footer);
        skipBtn->setObjectName("cbGhost");
        skipBtn->setCursor(Qt::PointingHandCursor);
        connect(skipBtn, &QPushButton::clicked, this, &ClassBoardSetupDialog::skip);
        row->addWidget(skipBtn);

        row->addStretch(1);

        QLabel* note = new QLabel(tr("Modifiable à tout moment dans le menu ClassBoard"), footer);
        note->setObjectName("cbHint");
        row->addWidget(note);

        row->addStretch(1);

        mPrimaryButton = new QPushButton(tr("Commencer →"), footer);
        mPrimaryButton->setObjectName("cbPrimary");
        mPrimaryButton->setDefault(true);
        mPrimaryButton->setCursor(Qt::PointingHandCursor);
        connect(mPrimaryButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::saveAndClose);
        row->addWidget(mPrimaryButton);
    }
    else
    {
        QPushButton* resetBtn = new QPushButton(tr("Réinitialiser"), footer);
        resetBtn->setObjectName("cbDanger");
        resetBtn->setCursor(Qt::PointingHandCursor);
        connect(resetBtn, &QPushButton::clicked, this, &ClassBoardSetupDialog::resetProfile);
        row->addWidget(resetBtn);

        row->addStretch(1);

        QPushButton* cancelBtn = new QPushButton(tr("Annuler"), footer);
        cancelBtn->setCursor(Qt::PointingHandCursor);
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        row->addWidget(cancelBtn);

        mPrimaryButton = new QPushButton(tr("Enregistrer"), footer);
        mPrimaryButton->setObjectName("cbPrimary");
        mPrimaryButton->setDefault(true);
        mPrimaryButton->setCursor(Qt::PointingHandCursor);
        connect(mPrimaryButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::saveAndClose);
        row->addWidget(mPrimaryButton);
    }

    return footer;
}

QStringList ClassBoardSetupDialog::selectedDisciplines() const
{
    QStringList ids;
    for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
    {
        QToolButton* button = mDisciplineButtons.value(d.id);
        if (button && button->isChecked())
            ids << d.id;
    }
    return ids;
}

void ClassBoardSetupDialog::loadFromProfile()
{
    const QStringList disciplines = ClassBoardProfile::disciplines();
    for (auto it = mDisciplineButtons.cbegin(); it != mDisciplineButtons.cend(); ++it)
        it.value()->setChecked(disciplines.contains(it.key()));

    const bool onLeft = ClassBoardProfile::toolbarSide() == ClassBoardProfile::ToolbarSide::Left;
    mSideLeft->setChecked(onLeft);
    mSideRight->setChecked(!onLeft);

    switch (ClassBoardProfile::theme())
    {
    case ClassBoardProfile::Theme::Dark:
        mThemeDark->setChecked(true);
        break;
    case ClassBoardProfile::Theme::System:
        mThemeSystem->setChecked(true);
        break;
    default:
        mThemeLight->setChecked(true);
        break;
    }

    const int scale = ClassBoardProfile::toolbarScale();
    mSize100->setChecked(scale < 115);
    mSize125->setChecked(scale >= 115 && scale < 140);
    mSize150->setChecked(scale >= 140);

    mShowLabels->setChecked(ClassBoardProfile::showToolLabels());

    mPendingDocsDir = ClassBoardProfile::documentsDirectory();
    if (mDocsDirLabel)
    {
        mDocsDirLabel->setText(mPendingDocsDir.isEmpty()
                               ? tr("Dossier local par défaut")
                               : QDir::toNativeSeparators(mPendingDocsDir));
    }
}

void ClassBoardSetupDialog::updateState()
{
    const int count = selectedDisciplines().size();
    if (count == 0)
        mSelectionSummary->setText(tr("Choisissez une ou plusieurs matières (ou cliquez sur Commencer pour un profil polyvalent)."));
    else if (count == 1)
        mSelectionSummary->setText(tr("1 matière choisie · Vos outils seront prêts dès l'ouverture."));
    else
        mSelectionSummary->setText(tr("%1 matières choisies · Vos outils seront prêts dès l'ouverture.").arg(count));
}

void ClassBoardSetupDialog::skip()
{
    ClassBoardProfile::setFirstRunCompleted(true);
    reject();
}

void ClassBoardSetupDialog::save()
{
    ClassBoardProfile::setDisciplines(selectedDisciplines());

    if (mMode == Mode::Settings)
    {
        ClassBoardProfile::setToolbarSide(mSideLeft->isChecked()
                                          ? ClassBoardProfile::ToolbarSide::Left
                                          : ClassBoardProfile::ToolbarSide::Right);

        ClassBoardProfile::Theme theme = ClassBoardProfile::Theme::Light;
        if (mThemeDark->isChecked())
            theme = ClassBoardProfile::Theme::Dark;
        else if (mThemeSystem->isChecked())
            theme = ClassBoardProfile::Theme::System;
        ClassBoardProfile::setTheme(theme);

        int scale = 125;
        if (mSize100->isChecked())
            scale = 100;
        else if (mSize150->isChecked())
            scale = 150;
        ClassBoardProfile::setToolbarScale(scale);
        ClassBoardProfile::setShowToolLabels(mShowLabels->isChecked());
        ClassBoardProfile::setDocumentsDirectory(mPendingDocsDir);
    }

    ClassBoardProfile::setFirstRunCompleted(true);
}

void ClassBoardSetupDialog::saveAndClose()
{
    save();
    accept();
}

void ClassBoardSetupDialog::resetProfile()
{
    const auto reply = QMessageBox::question(
        this,
        tr("Réinitialiser ClassBoard"),
        tr("Revenir aux réglages par défaut (matières, thème blanc, barre à droite) ?\n"
           "Vos cours et documents ne seront pas supprimés."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    ClassBoardProfile::reset();
    ClassBoardProfile::setFirstRunCompleted(true);
    loadFromProfile();
    updateState();
}
