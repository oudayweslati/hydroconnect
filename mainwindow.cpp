#include "mainwindow.h"
#include "gestion_equipement.h"
#include "logo-data.h"

#include <QButtonGroup>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {
const char *kSidebarStyle = R"CSS(
    QFrame#sidebar { background:#0D2B5E; }
    QPushButton#nav {
        color:#EAF1FF; background:transparent; border:none; border-radius:9px;
        text-align:left; padding:0 14px; font-size:12px; font-weight:600;
        min-height:36px; max-height:36px;
    }
    QPushButton#nav:hover { background:rgba(255,255,255,0.08); }
    QPushButton#nav:checked { background:#2F6FED; color:white; }
    QLabel#placeholder { color:#6C7C98; font-size:18px; background:transparent; }
)CSS";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("HydroConnect - Smart Water Factory (Integration Finale)");

    auto *central = new QWidget;
    central->setStyleSheet(kSidebarStyle);
    setCentralWidget(central);

    auto *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_stack = new QStackedWidget;
    root->addWidget(buildSidebar());
    root->addWidget(m_stack, 1);

    resize(1280, 740);
    setMinimumSize(1000, 600);
}

QWidget *MainWindow::buildSidebar()
{
    auto *side = new QFrame;
    side->setObjectName("sidebar");
    side->setFixedWidth(205);

    auto *layout = new QVBoxLayout(side);
    layout->setContentsMargins(8, 20, 8, 14);
    layout->setSpacing(4);

    // Logo + nom
    auto *logo = new QLabel;
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("background:transparent;");
    QPixmap pix;
    pix.loadFromData(kLogoPng, sizeof(kLogoPng), "PNG");
    if (!pix.isNull())
        logo->setPixmap(pix.scaledToWidth(150, Qt::SmoothTransformation));
    layout->addWidget(logo);

    layout->addSpacing(14);

    // Navigation : un bouton = une page
    const QStringList pages = {
        "Accueil", "Employes", "Produits", "Equipements", "Usines",
        "Clients", "Livraisons", "Statistiques", "Exportation", "Parametres"
    };

    auto *group = new QButtonGroup(this);
    group->setExclusive(true);

    for (int i = 0; i < pages.size(); ++i) {
        auto *button = new QPushButton(pages.at(i));
        button->setObjectName("nav");
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        group->addButton(button, i);
        layout->addWidget(button);

        // Seul le module Equipements est réalisé ici ; les autres sont des
        // pages vides à remplacer par les modules de l'équipe.
        if (pages.at(i) == "Equipements")
            m_stack->addWidget(new GestionEquipement);
        else
            m_stack->addWidget(makePlaceholder(pages.at(i)));

        connect(button, &QPushButton::clicked, this, [this, i]() {
            m_stack->setCurrentIndex(i);
        });

        if (pages.at(i) == "Equipements") {
            button->setChecked(true);
            m_stack->setCurrentIndex(i);
        }
    }

    layout->addStretch();
    return side;
}

QWidget *MainWindow::makePlaceholder(const QString &nom)
{
    auto *page = new QWidget;
    page->setStyleSheet("background:#F6F9FF;");
    auto *layout = new QVBoxLayout(page);
    auto *label = new QLabel(QString("Module « %1 »\nà intégrer ici").arg(nom));
    label->setObjectName("placeholder");
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    return page;
}
