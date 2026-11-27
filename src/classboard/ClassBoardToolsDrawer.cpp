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

#include "ClassBoardToolsDrawer.h"

#include "ClassBoardDisciplines.h"
#include "ClassBoardIcons.h"
#include "ClassBoardProfile.h"
#include "ClassBoardTheme.h"

#include "adaptors/UBExportPDF.h"
#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBMainWindow.h"

#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
    void clearLayoutWidgets(QLayout* layout)
    {
        if (!layout)
            return;

        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* w = item->widget())
                w->deleteLater();
            else if (QLayout* child = item->layout())
                clearLayoutWidgets(child);
            delete item;
        }
    }
}

ClassBoardToolsDrawer::ClassBoardToolsDrawer(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("cbToolsDrawer");
    setFixedWidth(416);

    buildCatalog();

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 16, 18, 16);
    root->setSpacing(12);

    // En-tête
    QHBoxLayout* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    QLabel* title = new QLabel(tr("Outils pédagogiques"), this);
    title->setObjectName("cbDrawerTitle");
    header->addWidget(title);
    header->addStretch(1);

    QToolButton* closeBtn = new QToolButton(this);
    closeBtn->setObjectName("cbCloseBtn");
    closeBtn->setIcon(ClassBoardIcons::icon("close", ClassBoardIcons::Style::Toolbar));
    closeBtn->setIconSize(QSize(18, 18));
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFocusPolicy(Qt::NoFocus);
    closeBtn->setToolTip(tr("Fermer"));
    connect(closeBtn, &QToolButton::clicked, this, [this]() {
        hide();
        emit closed();
    });
    header->addWidget(closeBtn);
    root->addLayout(header);

    // Recherche rapide
    mSearchEdit = new QLineEdit(this);
    mSearchEdit->setObjectName("cbSearchEdit");
    mSearchEdit->setPlaceholderText(tr("Rechercher un outil (minuteur, vocabulaire, rideau…)"));
    mSearchEdit->setClearButtonEnabled(true);
    connect(mSearchEdit, &QLineEdit::textChanged, this, [this]() { rebuildGrid(); });
    root->addWidget(mSearchEdit);

    // Filtres par discipline
    mChipHost = new QWidget(this);
    mChipLayout = new QHBoxLayout(mChipHost);
    mChipLayout->setContentsMargins(0, 0, 0, 0);
    mChipLayout->setSpacing(6);
    mChipGroup = new QButtonGroup(this);
    mChipGroup->setExclusive(true);
    root->addWidget(mChipHost);

    // Zone défilante des cartes d'outils
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setObjectName("cbDrawerScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mGridHost = new QWidget(scroll);
    mGridHost->setObjectName("cbDrawerGridHost");
    mGridLayout = new QVBoxLayout(mGridHost);
    mGridLayout->setContentsMargins(0, 2, 4, 4);
    mGridLayout->setSpacing(12);
    scroll->setWidget(mGridHost);
    root->addWidget(scroll, 1);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(36);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(15, 23, 42, 46));
    setGraphicsEffect(shadow);

    applyTheme();
    reloadForProfile();
}

void ClassBoardToolsDrawer::buildCatalog()
{
    mCatalog.clear();

    auto add = [this](const QString& id, const QString& label, const QString& keywords,
                      const QString& section, const QString& icon,
                      const QColor& fg, const QColor& bg,
                      ActionKind kind, const QString& target, const QStringList& tags)
    {
        mCatalog << ToolEntry{id, label, keywords, section, icon, fg, bg, kind, target, tags};
    };

    const QString sClasse = tr("CLASSE & ORAL");
    const QString sLangues = tr("ANGLAIS & FRANÇAIS");
    const QString sInserer = tr("INSÉRER & PRÉSENTER");
    const QString sMaths = tr("INSTRUMENTS & SCIENCES");

    // Classe & oral
    add("timer", tr("Minuteur"), "minuteur temps chronometre pair work oral", sClasse,
        "timer", QColor("#2563EB"), QColor("#DBEAFE"),
        ActionKind::Widget, "Minuteur", {"anglais", "langues", "francais", "classe", "histoire-geo", "general", "segpa", "ulis", "eps", "musique"});
    add("dice", tr("Tirage au sort"), "tirage sort eleve groupe binome de oral", sClasse,
        "dice", QColor("#7C3AED"), QColor("#EDE9FE"),
        ActionKind::Widget, "TirageAuSort", {"anglais", "langues", "francais", "classe", "histoire-geo", "general", "segpa", "ulis", "eps", "musique"});
    add("mask", tr("Rideau"), "rideau masque cacher reveler reponse traduction", sClasse,
        "mask", QColor("#0D9488"), QColor("#CCFBF1"),
        ActionKind::NativeTool, "openboardtool://openboard/mask", {"anglais", "langues", "francais", "classe", "histoire-geo", "svt", "general", "segpa", "ulis"});
    add("spotlight", tr("Projecteur"), "projecteur spot zoomer isoler texte", sClasse,
        "spotlight", QColor("#D97706"), QColor("#FEF3C7"),
        ActionKind::NativeTool, "openboardtool://cache", {"anglais", "langues", "francais", "classe", "histoire-geo", "svt", "general", "segpa", "ulis"});

    // Anglais & Français
    add("vocabulaire", tr("Vocabulaire"), "vocabulaire flashcards cartes anglais langue traduction memoire", sLangues,
        "vocabulaire", QColor("#4F46E5"), QColor("#EEF2FF"),
        ActionKind::Widget, "Vocabulaire", {"anglais", "langues", "francais", "general", "segpa", "ulis"});
    add("audio", tr("Lecteur audio"), "audio mp3 wav ecoute oral anglais langue musique vitesse boucle", sLangues,
        "audio", QColor("#0891B2"), QColor("#CFFAFE"),
        ActionKind::Widget, "LecteurAudio", {"anglais", "langues", "musique", "francais", "general", "classe"});
    add("etiquettes", tr("Étiquettes"), "etiquettes mots phrase ordre grammaire sujet verbe cod anglais francais", sLangues,
        "etiquettes", QColor("#0D9488"), QColor("#CCFBF1"),
        ActionKind::Widget, "Etiquettes", {"anglais", "langues", "francais", "general", "segpa", "ulis"});
    add("magnifier", tr("Loupe"), "loupe agrandir zoom texte detail", sLangues,
        "magnifier", QColor("#0284C7"), QColor("#E0F2FE"),
        ActionKind::NativeTool, "openboardtool://magnifier", {"francais", "histoire-geo", "svt", "arts-plastiques", "segpa", "ulis", "classe"});

    // Insérer, formes & présenter
    add("image", tr("Image"), "image photo illustrer inserer", sInserer,
        "image", QColor("#0284C7"), QColor("#E0F2FE"),
        ActionKind::ImportImage, "", {"anglais", "langues", "francais", "classe", "histoire-geo", "svt", "general"});
    add("pdf", tr("Importer PDF"), "pdf document fiche exercice manuel", sInserer,
        "pdf", QColor("#DC2626"), QColor("#FEE2E2"),
        ActionKind::ImportPdf, "", {"anglais", "langues", "francais", "classe", "histoire-geo", "math", "general"});
    add("export-pdf", tr("Exporter PDF"), "exporter pdf enregistrer pronote ent cours", sInserer,
        "export", QColor("#4F46E5"), QColor("#EEF2FF"),
        ActionKind::ExportPdf, "", {"anglais", "langues", "francais", "classe", "histoire-geo", "math", "general"});
    add("arrow", tr("Flèche"), "fleche relier montrer forme", sInserer,
        "arrow-right", QColor("#2563EB"), QColor("#DBEAFE"),
        ActionKind::ShapeSvg, "blue_arrow_right.svg", {"anglais", "langues", "francais", "classe", "histoire-geo", "svt", "general"});
    add("rect", tr("Encadré"), "encadre rectangle cadre regle grammaire forme", sInserer,
        "rect", QColor("#7C3AED"), QColor("#EDE9FE"),
        ActionKind::ShapeSvg, "nofill_rectangle_rnd.svg", {"anglais", "langues", "francais", "classe", "histoire-geo", "math", "general"});
    add("circle", tr("Cercle"), "cercle entourer forme", sInserer,
        "circle", QColor("#059669"), QColor("#D1FAE5"),
        ActionKind::ShapeSvg, "nofill_disk.svg", {"anglais", "francais", "classe", "math", "general"});
    add("capture", tr("Capture"), "capture ecran decouper zone", sInserer,
        "capture", QColor("#EA580C"), QColor("#FFEDD5"),
        ActionKind::Capture, "", {"anglais", "langues", "francais", "classe", "general"});
    add("line", tr("Trait droit"), "ligne trait souligner", sInserer,
        "line", QColor("#475569"), QColor("#F1F5F9"),
        ActionKind::LineTool, "", {"francais", "classe", "math", "histoire-geo", "general"});
    add("laser", tr("Laser"), "laser pointeur montrer presenter", sInserer,
        "laser", QColor("#E11D48"), QColor("#FFE4E6"),
        ActionKind::LaserTool, "", {"anglais", "francais", "classe", "histoire-geo", "svt", "general"});

    // Instruments & sciences (toujours disponibles sous « Tous » ou si discipline scientifique choisie)
    add("ruler", tr("Règle"), "regle mesurer tracer geometrie maths", sMaths,
        "ruler", QColor("#2563EB"), QColor("#DBEAFE"),
        ActionKind::NativeTool, "openboardtool://ruler", {"math", "physique-chimie", "technologie", "svt", "arts-plastiques"});
    add("triangle", tr("Équerre"), "equerre angle droit geometrie maths", sMaths,
        "triangle", QColor("#0D9488"), QColor("#CCFBF1"),
        ActionKind::NativeTool, "openboardtool://triangle", {"math", "technologie"});
    add("protractor", tr("Rapporteur"), "rapporteur angle degres geometrie maths", sMaths,
        "protractor", QColor("#7C3AED"), QColor("#EDE9FE"),
        ActionKind::NativeTool, "openboardtool://protractor", {"math", "physique-chimie", "technologie"});
    add("compass", tr("Compas"), "compas cercle tracer geometrie maths", sMaths,
        "compass", QColor("#D97706"), QColor("#FEF3C7"),
        ActionKind::NativeTool, "openboardtool://compass", {"math", "technologie", "arts-plastiques"});
    add("axes", tr("Repère"), "repere graphique coordonnees maths", sMaths,
        "axes", QColor("#4F46E5"), QColor("#EEF2FF"),
        ActionKind::NativeTool, "openboardtool://axes", {"math", "physique-chimie", "technologie"});
    add("calculator", tr("Calculatrice"), "calculatrice calcul maths sciences", sMaths,
        "calculator", QColor("#334155"), QColor("#E2E8F0"),
        ActionKind::Widget, "Calculator", {"math", "physique-chimie", "technologie"});
    add("stopwatch", tr("Chronomètre"), "chronometre temps eps sport musique", sMaths,
        "stopwatch", QColor("#059669"), QColor("#D1FAE5"),
        ActionKind::Widget, "Stopwatch", {"eps", "physique-chimie", "musique"});
    add("clock", tr("Horloge"), "horloge heure apprendre lire", sMaths,
        "clock", QColor("#0284C7"), QColor("#E0F2FE"),
        ActionKind::Widget, "Horloge", {"anglais", "langues", "eps", "musique", "segpa", "ulis"});
}

void ClassBoardToolsDrawer::applyTheme()
{
    const ClassBoardTheme::Colors c = ClassBoardTheme::colors();
    setStyleSheet(QStringLiteral(R"(
        QFrame#cbToolsDrawer {
            background: %1; border: 1px solid %2; border-radius: 24px;
        }
        QFrame#cbToolsDrawer QWidget#cbDrawerGridHost,
        QFrame#cbToolsDrawer QScrollArea#cbDrawerScroll {
            background: transparent;
        }
        QFrame#cbToolsDrawer QLabel#cbDrawerTitle {
            background: transparent; color: %3; font-size: 17px; font-weight: 800;
        }
        QFrame#cbToolsDrawer QLabel#cbSectionTitle {
            background: transparent; color: %4; font-size: 11px; font-weight: 700;
            letter-spacing: 0.5px; padding-top: 2px;
        }
        QFrame#cbToolsDrawer QLineEdit#cbSearchEdit {
            background: %5; color: %3; border: 1px solid %2; border-radius: 12px;
            padding: 8px 12px; font-size: 13px;
        }
        QFrame#cbToolsDrawer QLineEdit#cbSearchEdit:focus {
            border: 1.5px solid %6; background: %1;
        }
        QFrame#cbToolsDrawer QPushButton#cbChip {
            background: %5; color: %4; border: none; border-radius: 14px;
            padding: 6px 12px; font-size: 12px; font-weight: 700; min-height: 28px;
        }
        QFrame#cbToolsDrawer QPushButton#cbChip:hover {
            color: %3;
        }
        QFrame#cbToolsDrawer QPushButton#cbChip:checked {
            background: %6; color: %7;
        }
        QFrame#cbToolsDrawer QToolButton#cbCloseBtn {
            background: transparent; border: none; border-radius: 16px;
        }
        QFrame#cbToolsDrawer QToolButton#cbCloseBtn:hover {
            background: %5;
        }
        QFrame#cbToolsDrawer QToolButton#cbToolCard {
            background: %1; color: %3; border: 1px solid %2; border-radius: 16px;
            font-size: 12px; font-weight: 600; padding: 10px 4px 8px 4px;
        }
        QFrame#cbToolsDrawer QToolButton#cbToolCard:hover {
            background: %5; border-color: %6;
        }
    )").arg(ClassBoardTheme::colorName(c.surface),
            ClassBoardTheme::colorName(c.border),
            ClassBoardTheme::colorName(c.text),
            ClassBoardTheme::colorName(c.textSecondary),
            ClassBoardTheme::colorName(c.surfaceAlt),
            ClassBoardTheme::colorName(c.accent),
            ClassBoardTheme::colorName(c.accentText)));
}

void ClassBoardToolsDrawer::reloadForProfile()
{
    rebuildFilterChips();
    rebuildGrid();
}

void ClassBoardToolsDrawer::rebuildFilterChips()
{
    for (QAbstractButton* b : mChipGroup->buttons())
        mChipGroup->removeButton(b);
    clearLayoutWidgets(mChipLayout);

    struct Chip { QString id; QString label; };
    QList<Chip> chips;

    const QStringList userDisciplines = ClassBoardProfile::disciplines();
    for (const QString& id : userDisciplines)
    {
        if (chips.size() >= 2)
            break;
        const ClassBoardDiscipline d = ClassBoardDisciplines::byId(id);
        if (d.isValid())
        {
            QString shortName = d.displayName();
            if (id == "histoire-geo") shortName = tr("Histoire-Géo");
            else if (id == "langues") shortName = tr("Langues");
            else if (id == "general") shortName = tr("Polyvalent");
            else if (id == "physique-chimie") shortName = tr("Physique");
            else if (id == "arts-plastiques") shortName = tr("Arts");
            else if (id == "musique") shortName = tr("Musique");
            chips << Chip{id, shortName};
        }
    }

    if (chips.isEmpty())
        chips << Chip{"anglais", tr("Anglais")} << Chip{"francais", tr("Français")};

    auto hasChip = [&chips](const QString& id) {
        for (const Chip& c : chips)
            if (c.id == id) return true;
        return false;
    };
    if (!hasChip("classe"))
        chips << Chip{"classe", tr("Classe")};
    chips << Chip{"all", tr("Tous")};

    mActiveFilter = chips.first().id;

    for (int i = 0; i < chips.size(); ++i)
    {
        const Chip& c = chips.at(i);
        QPushButton* btn = new QPushButton(c.label, mChipHost);
        btn->setObjectName("cbChip");
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::NoFocus);
        if (i == 0)
            btn->setChecked(true);
        mChipGroup->addButton(btn, i);
        mChipLayout->addWidget(btn);

        const QString filterId = c.id;
        connect(btn, &QPushButton::clicked, this, [this, filterId]() {
            mActiveFilter = filterId;
            rebuildGrid();
        });
    }
    mChipLayout->addStretch(1);
}

void ClassBoardToolsDrawer::rebuildGrid()
{
    clearLayoutWidgets(mGridLayout);

    const QString query = mSearchEdit->text().trimmed().toLower();
    const bool searching = !query.isEmpty();
    const bool dark = ClassBoardTheme::isDark();

    QStringList sectionOrder;
    QHash<QString, QList<ToolEntry>> grouped;

    for (const ToolEntry& entry : mCatalog)
    {
        if (searching)
        {
            const QString hay = (entry.label + " " + entry.keywords).toLower();
            if (!hay.contains(query))
                continue;
        }
        else if (mActiveFilter != "all" && !entry.tags.contains(mActiveFilter))
        {
            continue;
        }

        if (!sectionOrder.contains(entry.section))
            sectionOrder << entry.section;
        grouped[entry.section] << entry;
    }

    if (sectionOrder.isEmpty())
    {
        QLabel* empty = new QLabel(tr("Aucun outil trouvé pour « %1 ».").arg(mSearchEdit->text()), mGridHost);
        empty->setStyleSheet("padding: 20px 6px; font-size: 13px;");
        empty->setAlignment(Qt::AlignCenter);
        mGridLayout->addWidget(empty);
        mGridLayout->addStretch(1);
        return;
    }

    for (const QString& section : sectionOrder)
    {
        QLabel* header = new QLabel(section, mGridHost);
        header->setObjectName("cbSectionTitle");
        mGridLayout->addWidget(header);

        QGridLayout* grid = new QGridLayout();
        grid->setHorizontalSpacing(10);
        grid->setVerticalSpacing(10);

        const QList<ToolEntry>& list = grouped[section];
        for (int i = 0; i < list.size(); ++i)
        {
            const ToolEntry entry = list.at(i);
            QToolButton* card = new QToolButton(mGridHost);
            card->setObjectName("cbToolCard");
            card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

            QColor bg = entry.bgColor;
            QColor fg = entry.fgColor;
            if (dark)
            {
                bg = QColor(fg.red(), fg.green(), fg.blue(), 46);
                fg = fg.lighter(135);
            }

            const QString res = QStringLiteral(":/classboard/tools/%1.svg").arg(entry.iconName);
            card->setIcon(ClassBoardTheme::duotoneIcon(res, fg, bg, 38, 12));
            card->setIconSize(QSize(38, 38));
            card->setText(entry.label);
            card->setFixedSize(118, 78);
            card->setCursor(Qt::PointingHandCursor);
            card->setFocusPolicy(Qt::NoFocus);

            connect(card, &QToolButton::clicked, this, [this, entry]() {
                launchEntry(entry);
            });

            grid->addWidget(card, i / 3, i % 3);
        }

        mGridLayout->addLayout(grid);
    }

    mGridLayout->addStretch(1);
}

void ClassBoardToolsDrawer::launchEntry(const ToolEntry& entry)
{
    UBBoardController* board = UBApplication::boardController;
    UBMainWindow* w = UBApplication::mainWindow;

    switch (entry.kind)
    {
    case ActionKind::NativeTool:
        if (board)
            board->downloadURL(QUrl(entry.target));
        break;

    case ActionKind::Widget:
        if (board)
        {
            const QString path = UBSettings::settings()->applicationApplicationsLibraryDirectory()
                                 + "/" + entry.target + ".wgt";
            if (QFile::exists(path))
                board->downloadURL(QUrl::fromLocalFile(path));
        }
        break;

    case ActionKind::ShapeSvg:
        if (board)
        {
            const QString path = UBSettings::settings()->applicationShapeLibraryDirectory()
                                 + "/" + entry.target;
            if (QFile::exists(path))
            {
                board->downloadURL(QUrl::fromLocalFile(path));
                UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
            }
        }
        break;

    case ActionKind::Capture:
        if (w && w->actionCapture)
            w->actionCapture->trigger();
        break;

    case ActionKind::LineTool:
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Line);
        break;

    case ActionKind::LaserTool:
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pointer);
        break;

    case ActionKind::ImportPdf:
        if (w && w->actionImportPage)
            w->actionImportPage->trigger();
        break;

    case ActionKind::ExportPdf:
        hide();
        emit closed();
        if (board && board->selectedDocument())
        {
            board->persistCurrentScene(false, true);
            UBExportPDF exporter;
            exporter.persist(board->selectedDocument());
        }
        emit toolLaunched();
        return;

    case ActionKind::ImportImage:
    {
        hide();
        emit closed();
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        const QString file = QFileDialog::getOpenFileName(
            w, tr("Insérer une image"), dir,
            tr("Images (*.png *.jpg *.jpeg *.webp *.bmp *.svg *.gif)"));
        if (!file.isEmpty() && board)
            board->downloadURL(QUrl::fromLocalFile(file));
        emit toolLaunched();
        return;
    }
    }

    hide();
    emit toolLaunched();
    emit closed();
}
