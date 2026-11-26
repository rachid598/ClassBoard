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
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    enum Page { DisciplinesPage = 0, AppearancePage = 1, FavoritesPage = 2, PageCount = 3 };

    // Coupe un libellé long en deux lignes pour les tuiles de discipline.
    QString wrapLabel(const QString& text)
    {
        if (text.size() <= 16)
            return text;

        const int middle = text.size() / 2;
        int best = -1;

        for (int i = 0; i < text.size(); ++i)
        {
            if (text.at(i) == ' ' && (best < 0 || qAbs(i - middle) < qAbs(best - middle)))
                best = i;
        }

        if (best < 0)
            return text;

        QString wrapped = text;
        wrapped[best] = '\n';
        return wrapped;
    }

    QIcon disciplineIcon(const QString& resource)
    {
        const ClassBoardTheme::Colors c = ClassBoardTheme::colors();
        QIcon icon;
        const QIcon off = ClassBoardTheme::tintedIcon(resource, c.text, 36);
        const QIcon on = ClassBoardTheme::tintedIcon(resource, c.accent, 36);

        for (const QSize& size : off.availableSizes())
        {
            icon.addPixmap(off.pixmap(size), QIcon::Normal, QIcon::Off);
            icon.addPixmap(on.pixmap(size), QIcon::Normal, QIcon::On);
        }

        return icon;
    }

    QStringList allKnownFavorites()
    {
        QStringList favorites;

        for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
        {
            for (const QString& f : d.favorites)
            {
                if (!favorites.contains(f))
                    favorites << f;
            }
        }

        return favorites;
    }
}

ClassBoardSetupDialog::ClassBoardSetupDialog(Mode mode, QWidget* parent)
    : QDialog(parent)
    , mMode(mode)
{
    setObjectName("ClassBoardSetupDialog");
    setWindowTitle(mode == Mode::FirstRun ? tr("Bienvenue dans ClassBoard") : tr("Mon profil ClassBoard"));
    setWindowIcon(QIcon(":/images/ClassBoard.png"));
    setStyleSheet(ClassBoardTheme::dialogStyleSheet());
    setModal(true);
    resize(1120, 780);

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 24);

    QWidget* card = new QWidget(this);
    card->setObjectName("cbCard");
    card->setAttribute(Qt::WA_StyledBackground, true);
    outer->addWidget(card);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(40, 32, 40, 28);
    layout->setSpacing(22);

    layout->addWidget(buildHeader());

    mPages = new QStackedWidget(card);
    mPages->addWidget(buildDisciplinesPage());
    mPages->addWidget(buildAppearancePage());
    mPages->addWidget(buildFavoritesPage());
    layout->addWidget(mPages, 1);

    layout->addWidget(buildFooter());

    loadFromProfile();
    updateState();
}

bool ClassBoardSetupDialog::runFirstLaunchIfNeeded()
{
    if (ClassBoardProfile::isFirstRunCompleted())
        return false;

    // Déploiements automatisés et tests : l'assistant peut être désactivé.
    if (qEnvironmentVariableIntValue("CLASSBOARD_SKIP_FIRST_RUN") == 1)
        return false;

    ClassBoardSetupDialog dialog(Mode::FirstRun);
    dialog.exec();

    // Même si l'enseignant ferme l'assistant, on ne le lui redemande pas :
    // il pourra toujours modifier ses choix dans "Mon profil ClassBoard".
    ClassBoardProfile::setFirstRunCompleted(true);
    return true;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

QWidget* ClassBoardSetupDialog::buildHeader()
{
    QWidget* header = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(header);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(12);

    QLabel* logo = new QLabel(header);
    logo->setPixmap(QIcon(":/images/ClassBoard.png").pixmap(44, 44));
    row->addWidget(logo);

    QLabel* name = new QLabel(mMode == Mode::FirstRun ? tr("Bienvenue dans ClassBoard") : tr("Mon profil ClassBoard"), header);
    name->setStyleSheet("font-size: 18px; font-weight: 700;");
    row->addWidget(name);
    row->addStretch(1);

    if (mMode == Mode::FirstRun)
    {
        mStepLabel = new QLabel(header);
        mStepLabel->setObjectName("cbStep");
        row->addWidget(mStepLabel);

        for (int i = 0; i < PageCount; ++i)
        {
            QLabel* bar = new QLabel(header);
            bar->setFixedSize(40, 6);
            row->addWidget(bar);
            mStepBars << bar;
        }
    }
    else
    {
        mNavGroup = new QButtonGroup(this);
        mNavGroup->setExclusive(true);
        const QStringList titles = {tr("Disciplines"), tr("Apparence"), tr("Favoris")};

        for (int i = 0; i < titles.size(); ++i)
        {
            QPushButton* button = new QPushButton(titles.at(i), header);
            button->setCheckable(true);
            button->setObjectName("cbNav");
            mNavGroup->addButton(button, i);
            row->addWidget(button);
        }

        mNavGroup->button(0)->setChecked(true);
        connect(mNavGroup, &QButtonGroup::idClicked, this, [this](int id) {
            if (id == FavoritesPage)
                refreshFavoritesIfUntouched();
            mPages->setCurrentIndex(id);
            updateState();
        });
    }

    return header;
}

QWidget* ClassBoardSetupDialog::buildDisciplinesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QLabel* title = new QLabel(tr("Quelle(s) discipline(s) enseignez-vous ?"), page);
    title->setObjectName("cbTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("ClassBoard affiche en priorité les outils et favoris adaptés. "
                                     "Tous les outils restent accessibles, quelle que soit la discipline."), page);
    subtitle->setObjectName("cbSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    layout->addSpacing(8);

    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(12);

    int index = 0;
    for (const ClassBoardDiscipline& d : ClassBoardDisciplines::all())
    {
        QToolButton* button = new QToolButton(page);
        button->setObjectName("cbDiscipline");
        button->setCheckable(true);
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIcon(disciplineIcon(d.icon));
        button->setIconSize(QSize(36, 36));
        button->setText(wrapLabel(d.displayName()));
        button->setToolTip(d.displayName());
        button->setAccessibleName(d.displayName());
        button->setMinimumSize(170, 112);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QToolButton::toggled, this, &ClassBoardSetupDialog::updateState);

        grid->addWidget(button, index / 5, index % 5);
        mDisciplineButtons.insert(d.id, button);
        ++index;
    }

    layout->addLayout(grid, 1);

    mSelectionSummary = new QLabel(page);
    mSelectionSummary->setObjectName("cbHint");
    layout->addWidget(mSelectionSummary);

    return page;
}

QWidget* ClassBoardSetupDialog::buildAppearancePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* title = new QLabel(tr("Apparence et barre d'outils"), page);
    title->setObjectName("cbTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Adaptez ClassBoard à votre salle : taille du tableau, luminosité du vidéoprojecteur..."), page);
    subtitle->setObjectName("cbSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    layout->addSpacing(14);

    QHBoxLayout* columns = new QHBoxLayout();
    columns->setSpacing(48);

    QVBoxLayout* themeColumn = new QVBoxLayout();
    QLabel* themeTitle = new QLabel(tr("Thème"), page);
    themeTitle->setObjectName("cbSection");
    themeColumn->addWidget(themeTitle);
    mThemeLight = new QRadioButton(tr("Clair"), page);
    mThemeDark = new QRadioButton(tr("Sombre (limite l'éblouissement au vidéoprojecteur)"), page);
    mThemeSystem = new QRadioButton(tr("Identique à Windows"), page);
    themeColumn->addWidget(mThemeLight);
    themeColumn->addWidget(mThemeDark);
    themeColumn->addWidget(mThemeSystem);
    themeColumn->addStretch(1);
    columns->addLayout(themeColumn);

    QVBoxLayout* sizeColumn = new QVBoxLayout();
    QLabel* sizeTitle = new QLabel(tr("Taille des boutons"), page);
    sizeTitle->setObjectName("cbSection");
    sizeColumn->addWidget(sizeTitle);
    mSize100 = new QRadioButton(tr("Standard"), page);
    mSize125 = new QRadioButton(tr("Grande (recommandée pour un TBI)"), page);
    mSize150 = new QRadioButton(tr("Très grande"), page);
    sizeColumn->addWidget(mSize100);
    sizeColumn->addWidget(mSize125);
    sizeColumn->addWidget(mSize150);
    sizeColumn->addSpacing(12);

    QLabel* optionsTitle = new QLabel(tr("Options"), page);
    optionsTitle->setObjectName("cbSection");
    sizeColumn->addWidget(optionsTitle);
    mShowLabels = new QCheckBox(tr("Afficher le nom des outils sous les icônes"), page);
    mShowFavoritesBar = new QCheckBox(tr("Afficher la barre de favoris à gauche du tableau"), page);
    sizeColumn->addWidget(mShowLabels);
    sizeColumn->addWidget(mShowFavoritesBar);
    sizeColumn->addStretch(1);
    columns->addLayout(sizeColumn);

    QButtonGroup* themeGroup = new QButtonGroup(page);
    themeGroup->addButton(mThemeLight);
    themeGroup->addButton(mThemeDark);
    themeGroup->addButton(mThemeSystem);

    QButtonGroup* sizeGroup = new QButtonGroup(page);
    sizeGroup->addButton(mSize100);
    sizeGroup->addButton(mSize125);
    sizeGroup->addButton(mSize150);

    themeColumn->setSpacing(4);
    sizeColumn->setSpacing(4);
    layout->addLayout(columns);
    layout->addStretch(1);

    QLabel* hint = new QLabel(tr("Le thème et la taille des boutons s'appliquent au prochain démarrage de ClassBoard."), page);
    hint->setObjectName("cbHint");
    layout->addWidget(hint);

    return page;
}

QWidget* ClassBoardSetupDialog::buildFavoritesPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    QLabel* title = new QLabel(tr("Vos outils favoris"), page);
    title->setObjectName("cbTitle");
    layout->addWidget(title);

    QLabel* subtitle = new QLabel(tr("Ils apparaissent dans la barre de favoris, à gauche du tableau. "
                                     "Cochez, décochez et faites glisser pour les réordonner."), page);
    subtitle->setObjectName("cbSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);
    layout->addSpacing(8);

    mFavorites = new QListWidget(page);
    mFavorites->setDragDropMode(QAbstractItemView::InternalMove);
    mFavorites->setDefaultDropAction(Qt::MoveAction);
    mFavorites->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(mFavorites, 1);

    connect(mFavorites, &QListWidget::itemChanged, this, [this]() {
        if (!mFillingFavorites)
            mFavoritesTouched = true;
    });
    connect(mFavorites->model(), &QAbstractItemModel::rowsMoved, this, [this]() {
        mFavoritesTouched = true;
    });

    QHBoxLayout* row = new QHBoxLayout();
    QPushButton* restore = new QPushButton(tr("Revenir aux favoris de mes disciplines"), page);
    connect(restore, &QPushButton::clicked, this, &ClassBoardSetupDialog::restoreDisciplineFavorites);
    row->addWidget(restore);
    row->addStretch(1);
    layout->addLayout(row);

    QLabel* where = new QLabel(tr("Vos choix sont enregistrés dans votre profil Windows : %1")
                                   .arg(QDir::toNativeSeparators(ClassBoardProfile::profileFile())), page);
    where->setObjectName("cbHint");
    where->setWordWrap(true);
    where->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(where);

    return page;
}

QWidget* ClassBoardSetupDialog::buildFooter()
{
    QWidget* footer = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(footer);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(12);

    if (mMode == Mode::FirstRun)
    {
        QLabel* note = new QLabel(tr("Modifiable à tout moment : menu ClassBoard › Mon profil ClassBoard."), footer);
        note->setObjectName("cbHint");
        row->addWidget(note);
        row->addStretch(1);

        mSkipButton = new QPushButton(tr("Passer"), footer);
        mBackButton = new QPushButton(tr("Précédent"), footer);
        mNextButton = new QPushButton(tr("Suivant"), footer);
        mNextButton->setObjectName("cbPrimary");
        mNextButton->setDefault(true);

        connect(mSkipButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::skip);
        connect(mBackButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::goBack);
        connect(mNextButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::goNext);

        row->addWidget(mSkipButton);
        row->addWidget(mBackButton);
        row->addWidget(mNextButton);
    }
    else
    {
        mResetButton = new QPushButton(tr("Réinitialiser ma configuration"), footer);
        mResetButton->setObjectName("cbDanger");
        connect(mResetButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::resetProfile);
        row->addWidget(mResetButton);
        row->addStretch(1);

        QPushButton* cancel = new QPushButton(tr("Annuler"), footer);
        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        row->addWidget(cancel);

        mNextButton = new QPushButton(tr("Enregistrer"), footer);
        mNextButton->setObjectName("cbPrimary");
        mNextButton->setDefault(true);
        connect(mNextButton, &QPushButton::clicked, this, &ClassBoardSetupDialog::saveAndClose);
        row->addWidget(mNextButton);
    }

    return footer;
}

// ---------------------------------------------------------------------------
// Données
// ---------------------------------------------------------------------------

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
    mShowFavoritesBar->setChecked(ClassBoardProfile::showFavoritesBar());

    fillFavorites(ClassBoardProfile::effectiveFavorites());
}

void ClassBoardSetupDialog::refreshFavoritesIfUntouched()
{
    if (mFavoritesTouched)
        return;

    const QStringList custom = ClassBoardProfile::customFavorites();
    fillFavorites(custom.isEmpty() ? ClassBoardDisciplines::favoritesFor(selectedDisciplines()) : custom);
}

void ClassBoardSetupDialog::fillFavorites(const QStringList& favorites)
{
    mFillingFavorites = true;
    mFavorites->clear();

    QStringList ordered = favorites;
    for (const QString& f : allKnownFavorites())
    {
        if (!ordered.contains(f))
            ordered << f;
    }

    for (const QString& f : ordered)
    {
        QListWidgetItem* item = new QListWidgetItem(ClassBoardDisciplines::favoriteDisplayName(f), mFavorites);
        item->setData(Qt::UserRole, f);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsDragEnabled);
        item->setCheckState(favorites.contains(f) ? Qt::Checked : Qt::Unchecked);
    }

    mFillingFavorites = false;
}

QStringList ClassBoardSetupDialog::checkedFavorites() const
{
    QStringList result;

    for (int i = 0; i < mFavorites->count(); ++i)
    {
        QListWidgetItem* item = mFavorites->item(i);

        if (item->checkState() == Qt::Checked)
            result << item->data(Qt::UserRole).toString();
    }

    return result;
}

void ClassBoardSetupDialog::restoreDisciplineFavorites()
{
    fillFavorites(ClassBoardDisciplines::favoritesFor(selectedDisciplines()));
    mFavoritesTouched = true;
}

void ClassBoardSetupDialog::save()
{
    const int oldScale = ClassBoardProfile::toolbarScale();
    const ClassBoardProfile::Theme oldTheme = ClassBoardProfile::theme();
    const bool oldLabels = ClassBoardProfile::showToolLabels();

    ClassBoardProfile::setDisciplines(selectedDisciplines());

    if (mThemeDark->isChecked())
        ClassBoardProfile::setTheme(ClassBoardProfile::Theme::Dark);
    else if (mThemeSystem->isChecked())
        ClassBoardProfile::setTheme(ClassBoardProfile::Theme::System);
    else
        ClassBoardProfile::setTheme(ClassBoardProfile::Theme::Light);

    ClassBoardProfile::setToolbarScale(mSize150->isChecked() ? 150 : (mSize125->isChecked() ? 125 : 100));
    ClassBoardProfile::setShowToolLabels(mShowLabels->isChecked());
    ClassBoardProfile::setShowFavoritesBar(mShowFavoritesBar->isChecked());

    // Favoris : une liste personnalisée n'est mémorisée que si l'enseignant a
    // modifié la liste ET qu'elle diffère de celle proposée par ses disciplines.
    // Sinon les favoris continuent de suivre les disciplines.
    if (mFavoritesTouched)
    {
        const QStringList favorites = checkedFavorites();
        const QStringList defaults = ClassBoardDisciplines::favoritesFor(selectedDisciplines());
        ClassBoardProfile::setCustomFavorites(favorites == defaults ? QStringList() : favorites);
    }

    mRestartRequired = oldScale != ClassBoardProfile::toolbarScale()
                    || oldTheme != ClassBoardProfile::theme()
                    || oldLabels != ClassBoardProfile::showToolLabels();
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

void ClassBoardSetupDialog::updateState()
{
    const QStringList selected = selectedDisciplines();

    if (mSelectionSummary)
    {
        if (selected.isEmpty())
            mSelectionSummary->setText(tr("Aucune discipline sélectionnée : les outils généraux seront proposés."));
        else if (selected.size() == 1)
            mSelectionSummary->setText(tr("1 discipline sélectionnée."));
        else
            mSelectionSummary->setText(tr("%1 disciplines sélectionnées.").arg(selected.size()));
    }

    const int page = mPages->currentIndex();

    if (mMode == Mode::FirstRun)
    {
        mStepLabel->setText(tr("Étape %1 sur %2").arg(page + 1).arg(PageCount));
        const ClassBoardTheme::Colors c = ClassBoardTheme::colors();

        for (int i = 0; i < mStepBars.size(); ++i)
        {
            const QColor color = i <= page ? c.accent : c.border;
            mStepBars.at(i)->setStyleSheet(QString("background: %1; border-radius: 3px;").arg(color.name()));
        }

        mBackButton->setVisible(page > 0);
        mNextButton->setText(page == PageCount - 1 ? tr("Commencer") : tr("Suivant"));
    }
}

void ClassBoardSetupDialog::goNext()
{
    const int page = mPages->currentIndex();

    if (page == DisciplinesPage)
    {
        // Les favoris proposés suivent les disciplines choisies
        refreshFavoritesIfUntouched();
    }

    if (page < PageCount - 1)
    {
        mPages->setCurrentIndex(page + 1);
        updateState();
        return;
    }

    saveAndClose();
}

void ClassBoardSetupDialog::goBack()
{
    const int page = mPages->currentIndex();

    if (page > 0)
    {
        mPages->setCurrentIndex(page - 1);
        updateState();
    }
}

void ClassBoardSetupDialog::skip()
{
    // Valeurs par défaut : outils généraux
    ClassBoardProfile::setFirstRunCompleted(true);
    reject();
}

void ClassBoardSetupDialog::saveAndClose()
{
    save();
    ClassBoardProfile::setFirstRunCompleted(true);

    if (mMode == Mode::Settings && mRestartRequired)
    {
        QMessageBox::information(this, tr("ClassBoard"),
                                 tr("Vos préférences sont enregistrées. Le thème et la taille des boutons "
                                    "seront appliqués au prochain démarrage de ClassBoard."));
    }

    accept();
}

void ClassBoardSetupDialog::resetProfile()
{
    const auto answer = QMessageBox::question(this, tr("Réinitialiser"),
                                              tr("Revenir à la configuration d'origine ? Vos disciplines, favoris et "
                                                 "préférences d'affichage seront effacés. Vos documents ne sont pas touchés."));

    if (answer != QMessageBox::Yes)
        return;

    ClassBoardProfile::reset();
    ClassBoardProfile::setFirstRunCompleted(true);
    mFavoritesTouched = false;
    loadFromProfile();
    updateState();
}
