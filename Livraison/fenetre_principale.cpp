#include "fenetre_principale.h"
#include "gestion_livraison.h"

#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

namespace {
// Index des pages (meme ordre que le menu).
enum Page { Accueil, Employes, Produits, Equipements, Usines, Clients,
            Livraisons, Statistiques, Exportation, Parametres };

struct Entree { const char *texte; };
const Entree kEntrees[] = {
    {"Accueil"}, {"Employes"}, {"Produits"}, {"Equipements"}, {"Usines"},
    {"Clients"}, {"Livraisons"}, {"Statistiques"}, {"Exportation"}, {"Parametres"},
};
}

FenetrePrincipale::FenetrePrincipale(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("HydroConnect"));

    auto *central = new QWidget;
    auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    h->addWidget(creerMenu());

    m_pages = new QStackedWidget;
    for (int i = 0; i < 10; ++i) {
        if (i == Livraisons)
            m_pages->addWidget(new GestionLivraison);                 // module realise
        else
            m_pages->addWidget(creerPageVide(QString::fromUtf8(kEntrees[i].texte)));
    }
    h->addWidget(m_pages, 1);
    setCentralWidget(central);

    afficherPage(Livraisons);
}

QWidget *FenetrePrincipale::creerMenu()
{
    auto *menu = new QFrame;
    menu->setObjectName("menuLateral");
    menu->setFixedWidth(238);
    menu->setStyleSheet(
        "QFrame#menuLateral{background:#0D2B5E;}"
        "QLabel#logo{background:transparent;}"
        "QPushButton{background:transparent;color:white;border:0;border-radius:12px;"
        "text-align:left;padding:11px 18px;font-size:11pt;}"
        "QPushButton:hover{background:#17407F;}"
        "QPushButton:checked{background:#2F6FED;}");

    auto *v = new QVBoxLayout(menu);
    v->setContentsMargins(0, 10, 0, 16);
    v->setSpacing(4);

    auto *logo = new QLabel;
    logo->setObjectName("logo");
    logo->setAlignment(Qt::AlignCenter);
    const QPixmap pm(":/images/logo.png");
    if (!pm.isNull())
        logo->setPixmap(pm.scaledToWidth(220, Qt::SmoothTransformation));
    else
        logo->setText("<b style='color:white;font-size:16pt'>HydroConnect</b>");
    v->addWidget(logo);
    v->addSpacing(8);

    auto *groupe = new QButtonGroup(this);
    groupe->setExclusive(true);
    for (int i = 0; i < 10; ++i) {
        QPushButton *b = creerBouton(QString::fromUtf8(kEntrees[i].texte), i);
        groupe->addButton(b, i);
        m_boutons.append(b);
        auto *ligne = new QHBoxLayout;
        ligne->setContentsMargins(2, 0, 2, 0);
        ligne->addWidget(b);
        v->addLayout(ligne);
    }
    v->addStretch();
    return menu;
}

QPushButton *FenetrePrincipale::creerBouton(const QString &texte, int index)
{
    auto *b = new QPushButton(texte);
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    connect(b, &QPushButton::clicked, this, [this, index]() { afficherPage(index); });
    return b;
}

QWidget *FenetrePrincipale::creerPageVide(const QString &titre)
{
    auto *page = new QWidget;
    page->setObjectName("pageVide");
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setStyleSheet("QWidget#pageVide{background:#F3F8FE;}"
                        "QLabel#titreVide{color:#103A70;font-size:23pt;font-weight:800;}"
                        "QLabel#texteVide{color:#52749B;font-size:11pt;}");
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(22, 18, 22, 18);
    auto *t = new QLabel(titre);
    t->setObjectName("titreVide");
    auto *s = new QLabel(tr("Ce module est vide pour le moment."));
    s->setObjectName("texteVide");
    v->addWidget(t);
    v->addWidget(s);
    v->addStretch();
    return page;
}

void FenetrePrincipale::afficherPage(int index)
{
    if (index < 0 || index >= m_pages->count()) return;
    m_pages->setCurrentIndex(index);
    if (index < m_boutons.size()) m_boutons[index]->setChecked(true);
}
