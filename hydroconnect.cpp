#include "hydroconnect.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPrinter>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>
#include <QFormLayout>
#include <algorithm>

// ---------------------------------------------------------------- construction
HydroConnect::HydroConnect(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("HydroConnect - Smart Water Factory");
    resize(1365, 723);

    // Theme bleu (menu #0b2a55 au lieu de noir)
    QFile qss(":/theme_hydroconnect.qss");
    if (qss.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(qss.readAll()));

    auto *central = new QWidget(this);
    central->setObjectName("page");
    auto *h = new QHBoxLayout(central);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);

    h->addWidget(creerMenu());

    m_stack = new QStackedWidget;
    m_stack->addWidget(creerPageUsines());
    auto *autre = new QLabel("Module en cours de developpement");
    autre->setAlignment(Qt::AlignCenter);
    m_stack->addWidget(autre);
    h->addWidget(m_stack, 1);

    setCentralWidget(central);
    rafraichir();
}

// ---------------------------------------------------------------- menu lateral
QWidget *HydroConnect::creerMenu()
{
    auto *side = new QFrame;
    side->setObjectName("sidebar");
    side->setFixedWidth(212);
    auto *v = new QVBoxLayout(side);
    v->setContentsMargins(10, 24, 10, 10);
    v->setSpacing(4);

    auto *logo = new QLabel;
    logo->setPixmap(QPixmap(":/logo_hydroconnect.png").scaledToWidth(110, Qt::SmoothTransformation));
    logo->setAlignment(Qt::AlignCenter);
    v->addWidget(logo);

    auto *txt = new QLabel("HydroConnect");
    txt->setObjectName("logoText");
    txt->setAlignment(Qt::AlignCenter);
    v->addWidget(txt);
    v->addSpacing(30);

    auto *grp = new QButtonGroup(this);
    grp->setExclusive(true);
    const QStringList items = {"Accueil", "Employes", "Produits", "Equipements", "Usines",
                               "Clients", "Livraisons", "Statistiques", "Exportation", "Parametres"};
    for (const QString &nom : items) {
        auto *b = new QPushButton(nom);
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setMinimumHeight(40);
        if (nom == "Usines") b->setChecked(true);
        grp->addButton(b);
        v->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, nom]() {
            m_stack->setCurrentIndex(nom == "Usines" ? 0 : 1);
        });
    }
    v->addStretch();
    return side;
}

QWidget *HydroConnect::creerCarte(const QString &tag, const QString &bg, const QString &fg,
                                  const QString &titre, QLabel *&valeur, const QString &sousTitre)
{
    auto *card = new QFrame;
    card->setObjectName("card");
    auto *l = new QHBoxLayout(card);
    l->setContentsMargins(20, 16, 20, 16);

    auto *t = new QLabel(tag);
    t->setObjectName("tag");
    t->setStyleSheet(QString("background:%1; color:%2;").arg(bg, fg));
    l->addWidget(t);

    auto *col = new QVBoxLayout;
    auto *lt = new QLabel(titre);   lt->setObjectName("cardTitle");
    valeur = new QLabel("0");       valeur->setObjectName("cardValue");
    auto *ls = new QLabel(sousTitre); ls->setObjectName("cardSub");
    col->addWidget(lt); col->addWidget(valeur); col->addWidget(ls);
    l->addLayout(col);
    l->addStretch();
    return card;
}

// ---------------------------------------------------------------- page Usines
QWidget *HydroConnect::creerPageUsines()
{
    auto *page = new QWidget;
    page->setObjectName("page");
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(20, 18, 20, 12);
    v->setSpacing(12);

    auto *titre = new QLabel("GESTION DES USINES");
    titre->setObjectName("pageTitle");
    auto *sous = new QLabel("Plateforme HydroConnect - Suivi de production et performance hydrique");
    sous->setObjectName("pageSub");
    v->addWidget(titre);
    v->addWidget(sous);

    // Cartes statistiques
    auto *cards = new QHBoxLayout;
    cards->setSpacing(12);
    cards->addWidget(creerCarte("USINES", "#e8f0ff", "#2f6fed", "NOMBRE D'USINES", m_lblNb, "Sites industriels"));
    cards->addWidget(creerCarte("ACTIF", "#d9f7ec", "#12a06b", "OPERATIONNELLES", m_lblOp, "En service actif"));
    cards->addWidget(creerCarte("MAINT.", "#fff0d6", "#e08a00", "EN MAINTENANCE", m_lblMaint, "Entretien en cours"));
    cards->addWidget(creerCarte("DEBIT", "#e8f0ff", "#2f6fed", "DEBIT GLOBAL", m_lblDebit, "Debit total cumule"));
    v->addLayout(cards);

    // Barre d'outils
    auto *bar = new QHBoxLayout;
    m_recherche = new QLineEdit;
    m_recherche->setPlaceholderText("Rechercher nom, localisation, type...");
    m_recherche->setMinimumWidth(230);
    m_tri = new QComboBox;
    m_tri->addItems({"Tri : Defaut", "Tri : Nom", "Tri : Capacite", "Tri : Debit", "Tri : Consommation"});
    m_filtreEtat = new QComboBox;
    m_filtreEtat->addItems({"Etat : Tous", "Operationnel", "En maintenance", "Arret technique"});
    bar->addWidget(m_recherche);
    bar->addWidget(m_tri);
    bar->addWidget(m_filtreEtat);
    bar->addStretch();

    auto bouton = [&](const QString &txt, const char *style, auto slot) {
        auto *b = new QPushButton(txt);
        b->setObjectName(style);
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, slot);
        bar->addWidget(b);
    };
    bouton("+ Nouvelle Usine", "primary", &HydroConnect::ajouterUsine);
    bouton("Modifier", "soft", &HydroConnect::modifierUsine);
    bouton("Supprimer", "outline", &HydroConnect::supprimerUsine);
    bouton("Exporter Excel", "soft", &HydroConnect::exporterExcel);
    bouton("Exporter PDF", "outline", &HydroConnect::exporterPdf);
    v->addLayout(bar);

    connect(m_recherche, &QLineEdit::textChanged, this, &HydroConnect::rafraichir);
    connect(m_tri, &QComboBox::currentIndexChanged, this, &HydroConnect::rafraichir);
    connect(m_filtreEtat, &QComboBox::currentIndexChanged, this, &HydroConnect::rafraichir);

    // Tableau
    auto *panelT = new QFrame;
    panelT->setObjectName("panel");
    auto *lt = new QVBoxLayout(panelT);
    lt->setContentsMargins(10, 10, 10, 10);
    m_table = new QTableWidget(0, 8);
    m_table->setHorizontalHeaderLabels({"ID", "Nom de l'Usine", "Localisation", "Type Traitement",
                                        "Capacite (m3)", "Debit (m3/h)", "Consommation (kWh)", "Etat"});
    m_table->verticalHeader()->hide();
    m_table->setShowGrid(false);
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setMinimumHeight(200);
    lt->addWidget(m_table);
    v->addWidget(panelT, 1);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &HydroConnect::ligneSelectionnee);

    // Bas : formulaire + onglets
    auto *bas = new QHBoxLayout;
    bas->setSpacing(12);

    auto *fiche = new QFrame;
    fiche->setObjectName("panel");
    fiche->setFixedWidth(320);
    auto *fl = new QVBoxLayout(fiche);
    auto *ft = new QLabel("Fiche Usine / Edition Rapide");
    ft->setStyleSheet("font-weight:bold; font-size:10pt; background:transparent;");
    fl->addWidget(ft);
    auto *form = new QFormLayout;
    m_fId = new QSpinBox;        m_fId->setRange(1, 999999);
    m_fNom = new QLineEdit;
    m_fLoc = new QLineEdit;
    m_fType = new QComboBox;
    m_fType->addItems({"Traitement Eau Potable", "Dessalement Eau de Mer", "Epuration Eaux Usees",
                       "Filtration Eau Douce", "Traitement Industriel"});
    m_fCap = new QDoubleSpinBox;   m_fCap->setRange(0, 1e7);   m_fCap->setSuffix(" m3");
    m_fDebit = new QDoubleSpinBox; m_fDebit->setRange(0, 1e6); m_fDebit->setSuffix(" m3/h");
    m_fConso = new QDoubleSpinBox; m_fConso->setRange(0, 1e7); m_fConso->setSuffix(" kWh");
    m_fEtat = new QComboBox;
    m_fEtat->addItems({"Operationnel", "En maintenance", "Arret technique"});
    form->addRow("ID :", m_fId);
    form->addRow("Nom :", m_fNom);
    form->addRow("Localisation :", m_fLoc);
    form->addRow("Type :", m_fType);
    form->addRow("Capacite :", m_fCap);
    form->addRow("Debit :", m_fDebit);
    form->addRow("Conso :", m_fConso);
    form->addRow("Etat :", m_fEtat);
    fl->addLayout(form);
    fl->addStretch();
    bas->addWidget(fiche);

    auto *tabs = new QTabWidget;
    tabs->setMinimumHeight(240);

    // Simulateur EcoWater
    auto *sim = new QWidget;
    auto *sl = new QFormLayout(sim);
    m_sDebit = new QDoubleSpinBox;  m_sDebit->setRange(0, 1e6);  m_sDebit->setValue(1000); m_sDebit->setSuffix(" m3/h");
    m_sHeures = new QDoubleSpinBox; m_sHeures->setRange(0, 24);  m_sHeures->setValue(20);  m_sHeures->setSuffix(" h/jour");
    m_sKwh = new QDoubleSpinBox;    m_sKwh->setRange(0, 100);    m_sKwh->setValue(0.8);    m_sKwh->setSuffix(" kWh/m3");
    m_sPrix = new QDoubleSpinBox;   m_sPrix->setRange(0, 10);    m_sPrix->setDecimals(3);  m_sPrix->setValue(0.25); m_sPrix->setSuffix(" TND/kWh");
    m_sResultat = new QLabel("Cliquez sur Simuler.");
    auto *bs = new QPushButton("Simuler");
    bs->setObjectName("primary");
    sl->addRow("Debit :", m_sDebit);
    sl->addRow("Fonctionnement :", m_sHeures);
    sl->addRow("Intensite energetique :", m_sKwh);
    sl->addRow("Prix energie :", m_sPrix);
    sl->addRow(bs);
    sl->addRow(m_sResultat);
    connect(bs, &QPushButton::clicked, this, &HydroConnect::simuler);
    tabs->addTab(sim, "Simulateur EcoWater");

    // Assistant IA
    auto *ia = new QWidget;
    auto *il = new QVBoxLayout(ia);
    auto *rapides = new QHBoxLayout;
    const QStringList suggestions = {"Etat global", "Alertes actives", "Conso energie", "Conseil IA"};
    for (const QString &k : suggestions) {
        auto *b = new QPushButton(k);
        b->setObjectName("soft");
        connect(b, &QPushButton::clicked, this, [this, k]() { reponseRapide(k); });
        rapides->addWidget(b);
    }
    rapides->addStretch();
    il->addLayout(rapides);
    m_chat = new QTextBrowser;
    m_chat->setHtml("<b style='color:#2f6fed'>Assistant HydroConnect IA :</b> Bonjour ! Je suis votre copilote "
                    "intelligent pour la gestion des usines de traitement d'eau. Posez-moi une question "
                    "ci-dessous ou cliquez sur une suggestion rapide.");
    il->addWidget(m_chat, 1);
    m_question = new QLineEdit;
    m_question->setPlaceholderText("Posez votre question...");
    il->addWidget(m_question);
    connect(m_question, &QLineEdit::returnPressed, this, &HydroConnect::questionIA);
    tabs->addTab(ia, "Assistant IA HydroConnect");
    tabs->setCurrentIndex(1);

    bas->addWidget(tabs, 1);
    v->addLayout(bas);
    return page;
}

// ---------------------------------------------------------------- donnees
QString HydroConnect::etatCouleur(const QString &etat) const
{
    if (etat == "Operationnel")    return "#12a06b";
    if (etat == "En maintenance")  return "#e08a00";
    return "#e03131";
}

void HydroConnect::rafraichir()
{
    if (!m_table) return;
    m_toutes = Usine::tous();

    const QString mot = m_recherche->text().trimmed();
    const QString etat = m_filtreEtat->currentIndex() == 0 ? QString() : m_filtreEtat->currentText();

    m_usines.clear();
    for (const Usine &u : std::as_const(m_toutes)) {
        const bool okMot = mot.isEmpty()
        || u.nom.contains(mot, Qt::CaseInsensitive)
            || u.localisation.contains(mot, Qt::CaseInsensitive)
            || u.type.contains(mot, Qt::CaseInsensitive);
        const bool okEtat = etat.isEmpty() || u.etat == etat;
        if (okMot && okEtat) m_usines.append(u);
    }

    switch (m_tri->currentIndex()) {
    case 1: std::sort(m_usines.begin(), m_usines.end(), [](const Usine &a, const Usine &b){ return a.nom < b.nom; }); break;
    case 2: std::sort(m_usines.begin(), m_usines.end(), [](const Usine &a, const Usine &b){ return a.capacite > b.capacite; }); break;
    case 3: std::sort(m_usines.begin(), m_usines.end(), [](const Usine &a, const Usine &b){ return a.debit > b.debit; }); break;
    case 4: std::sort(m_usines.begin(), m_usines.end(), [](const Usine &a, const Usine &b){ return a.conso > b.conso; }); break;
    default: break;
    }

    m_table->setRowCount(m_usines.size());
    for (int r = 0; r < m_usines.size(); ++r) {
        const Usine &u = m_usines[r];
        const QStringList cols = {QString::number(u.id), u.nom, u.localisation, u.type,
                                  QString::number(u.capacite, 'f', 0) + " m3",
                                  QString::number(u.debit, 'f', 0) + " m3/h",
                                  QString::number(u.conso, 'f', 0) + " kWh", u.etat};
        for (int c = 0; c < cols.size(); ++c) {
            auto *it = new QTableWidgetItem(cols[c]);
            if (c == 7) it->setForeground(QColor(etatCouleur(u.etat)));
            m_table->setItem(r, c, it);
        }
    }

    // Statistiques globales
    int op = 0, maint = 0;
    double debit = 0;
    for (const Usine &u : std::as_const(m_toutes)) {
        if (u.etat == "Operationnel") ++op;
        if (u.etat == "En maintenance") ++maint;
        debit += u.debit;
    }
    m_lblNb->setText(QString::number(m_toutes.size()));
    m_lblOp->setText(QString::number(op));
    m_lblMaint->setText(QString::number(maint));
    m_lblDebit->setText(QString::number(debit, 'f', 0) + " m3/h");
}

Usine HydroConnect::usineDepuisFormulaire() const
{
    return Usine(m_fId->value(), m_fNom->text().trimmed(), m_fLoc->text().trimmed(),
                 m_fType->currentText(), m_fCap->value(), m_fDebit->value(),
                 m_fConso->value(), m_fEtat->currentText());
}

void HydroConnect::ligneSelectionnee()
{
    const int r = m_table->currentRow();
    if (r < 0 || r >= m_usines.size()) return;
    const Usine &u = m_usines[r];
    m_fId->setValue(u.id);
    m_fNom->setText(u.nom);
    m_fLoc->setText(u.localisation);
    m_fType->setCurrentText(u.type);
    m_fCap->setValue(u.capacite);
    m_fDebit->setValue(u.debit);
    m_fConso->setValue(u.conso);
    m_fEtat->setCurrentText(u.etat);
}

void HydroConnect::ajouterUsine()
{
    Usine u = usineDepuisFormulaire();
    if (u.nom.isEmpty() || u.localisation.isEmpty()) {
        QMessageBox::warning(this, "Nouvelle usine", "Veuillez renseigner le nom et la localisation.");
        return;
    }
    if (Usine::existe(u.id)) u.id = Usine::prochainId();   // ID deja pris : on en attribue un nouveau
    if (u.ajouter()) {
        m_fId->setValue(u.id);
        rafraichir();
    } else {
        QMessageBox::critical(this, "Nouvelle usine", "Echec de l'ajout.\n" + Usine::derniereErreur());
    }
}

void HydroConnect::modifierUsine()
{
    const Usine u = usineDepuisFormulaire();
    if (!Usine::existe(u.id)) {
        QMessageBox::warning(this, "Modifier", "Selectionnez d'abord une usine existante.");
        return;
    }
    if (u.modifier()) rafraichir();
    else QMessageBox::critical(this, "Modifier", "Echec de la modification.\n" + Usine::derniereErreur());
}

void HydroConnect::supprimerUsine()
{
    const int id = m_fId->value();
    if (!Usine::existe(id)) {
        QMessageBox::warning(this, "Supprimer", "Selectionnez d'abord une usine.");
        return;
    }
    if (QMessageBox::question(this, "Supprimer", QString("Supprimer l'usine %1 ?").arg(id)) != QMessageBox::Yes)
        return;
    if (Usine::supprimer(id)) rafraichir();
    else QMessageBox::critical(this, "Supprimer", "Echec de la suppression.\n" + Usine::derniereErreur());
}

// ---------------------------------------------------------------- exports
void HydroConnect::exporterExcel()
{
    const QString path = QFileDialog::getSaveFileName(this, "Exporter Excel", "usines.csv", "CSV (*.csv)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF" << "ID;Nom;Localisation;Type;Capacite (m3);Debit (m3/h);Consommation (kWh);Etat\n";
    for (const Usine &u : std::as_const(m_usines))
        out << u.id << ';' << u.nom << ';' << u.localisation << ';' << u.type << ';'
            << u.capacite << ';' << u.debit << ';' << u.conso << ';' << u.etat << '\n';
    QMessageBox::information(this, "Export", "Export termine.");
}

void HydroConnect::exporterPdf()
{
    const QString path = QFileDialog::getSaveFileName(this, "Exporter PDF", "usines.pdf", "PDF (*.pdf)");
    if (path.isEmpty()) return;
    QString html = "<h2 style='color:#0b2a55'>HydroConnect - Liste des usines</h2>"
                   "<table border='1' cellspacing='0' cellpadding='4' width='100%'>"
                   "<tr style='background:#e8f0ff'><th>ID</th><th>Nom</th><th>Localisation</th><th>Type</th>"
                   "<th>Capacite</th><th>Debit</th><th>Conso</th><th>Etat</th></tr>";
    for (const Usine &u : std::as_const(m_usines))
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5 m3</td><td>%6 m3/h</td>"
                        "<td>%7 kWh</td><td>%8</td></tr>")
                    .arg(u.id).arg(u.nom, u.localisation, u.type)
                    .arg(u.capacite, 0, 'f', 0).arg(u.debit, 0, 'f', 0).arg(u.conso, 0, 'f', 0).arg(u.etat);
    html += "</table>";
    QTextDocument doc;
    doc.setHtml(html);
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    doc.print(&printer);
    QMessageBox::information(this, "Export", "PDF genere.");
}

// ---------------------------------------------------------------- simulateur + IA
void HydroConnect::simuler()
{
    const double volume = m_sDebit->value() * m_sHeures->value();
    const double kwh = volume * m_sKwh->value();
    const double cout = kwh * m_sPrix->value();
    m_sResultat->setText(QString("Production : %1 m3/jour  |  Energie : %2 kWh/jour  |  Cout : %3 TND/jour")
                             .arg(volume, 0, 'f', 0).arg(kwh, 0, 'f', 0).arg(cout, 0, 'f', 1));
}

QString HydroConnect::reponseIA(const QString &cle) const
{
    const QString k = cle.toLower();
    if (m_toutes.isEmpty()) return "Aucune usine enregistree.";

    if (k.contains("etat") || k.contains("global")) {
        int op = 0, mt = 0, at = 0; double deb = 0;
        for (const Usine &u : m_toutes) {
            if (u.etat == "Operationnel") ++op; else if (u.etat == "En maintenance") ++mt; else ++at;
            deb += u.debit;
        }
        return QString("%1 usines : %2 operationnelles, %3 en maintenance, %4 en arret technique. "
                       "Debit cumule : %5 m3/h.").arg(m_toutes.size()).arg(op).arg(mt).arg(at).arg(deb, 0, 'f', 0);
    }
    if (k.contains("alerte")) {
        QStringList a;
        for (const Usine &u : m_toutes)
            if (u.etat != "Operationnel") a << QString("%1 (%2)").arg(u.nom, u.etat);
        return a.isEmpty() ? "Aucune alerte active." : "Alertes actives : " + a.join(", ") + ".";
    }
    if (k.contains("conso") || k.contains("energie")) {
        double tot = 0; const Usine *max = &m_toutes[0];
        for (const Usine &u : m_toutes) { tot += u.conso; if (u.conso > max->conso) max = &u; }
        return QString("Consommation totale : %1 kWh. Plus gros consommateur : %2 (%3 kWh).")
            .arg(tot, 0, 'f', 0).arg(max->nom).arg(max->conso, 0, 'f', 0);
    }
    if (k.contains("conseil")) {
        const Usine *pire = nullptr; double ratio = 0;
        for (const Usine &u : m_toutes)
            if (u.debit > 0 && u.conso / u.debit > ratio) { ratio = u.conso / u.debit; pire = &u; }
        QString s;
        if (pire) s = QString("Optimisez en priorite %1 : %2 kWh par m3/h, le ratio le plus eleve. ")
                    .arg(pire->nom).arg(ratio, 0, 'f', 2);
        for (const Usine &u : m_toutes)
            if (u.etat == "Arret technique") s += QString("Planifiez la remise en service de %1. ").arg(u.nom);
        return s.isEmpty() ? "Tout fonctionne correctement." : s.trimmed();
    }
    return "Je peux vous renseigner sur l'etat global, les alertes, la consommation d'energie ou donner un conseil.";
}

void HydroConnect::reponseRapide(const QString &cle)
{
    m_chat->append(QString("<br><b>Vous :</b> %1").arg(cle.toHtmlEscaped()));
    m_chat->append(QString("<b style='color:#2f6fed'>Assistant HydroConnect IA :</b> %1")
                       .arg(reponseIA(cle).toHtmlEscaped()));
}

void HydroConnect::questionIA()
{
    const QString q = m_question->text().trimmed();
    if (q.isEmpty()) return;
    m_question->clear();
    reponseRapide(q);
}