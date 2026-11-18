#include "gestion_livraison.h"
#include "livraison_donutwidget.h"
#include "connection.h"

#include <QSqlQueryModel>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QIntValidator>
#include <QDateEdit>
#include <QProgressBar>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QStringConverter>
#include <QTime>
#include <QShowEvent>
#include <QDate>
#include <QColor>
#include <QPalette>

// ---------- Helpers de style ----------
static QLabel *fieldLabel(const QString &s) { auto *l = new QLabel(s); l->setObjectName("fieldLabel"); return l; }
static QPushButton *actionButton(const QString &text, const QString &color) {
    auto *b = new QPushButton(text);
    b->setMinimumHeight(38);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QPushButton{background:%1;color:white;border:0;border-radius:7px;padding:8px 13px;font-weight:600;} QPushButton:hover{background:#0B2A55;}").arg(color));
    return b;
}
static QFrame *cardFrame() { auto *f = new QFrame; f->setObjectName("card"); return f; }
static QLabel *cardTitle(const QString &s) { auto *l = new QLabel(s); l->setObjectName("cardTitle"); return l; }

static void statusColors(const QString &code, QColor &bg, QColor &fg)
{
    bg = QColor("#DBEAFE"); fg = QColor("#1D4ED8");                       // PLANIFIEE
    if (code == "LIVREE")         { bg = QColor("#D1FAE5"); fg = QColor("#047857"); }
    else if (code == "EN_RETARD") { bg = QColor("#FEE2E2"); fg = QColor("#B91C1C"); }
    else if (code == "EN_COURS")  { bg = QColor("#FEF3C7"); fg = QColor("#B45309"); }
}

// Une livraison est en retard si son statut est EN_RETARD, ou si la date prevue
// est depassee alors qu'elle n'est pas encore livree (detection automatique).
static bool isLate(const QModelIndex &index)
{
    const QAbstractItemModel *m = index.model();
    if (!m) return false;
    const QString code = m->index(index.row(), 6).data().toString();
    if (code == "EN_RETARD") return true;
    if (code == "LIVREE") return false;
    const QDate d = QDate::fromString(m->index(index.row(), 4).data().toString(), "dd/MM/yyyy");
    return d.isValid() && d < QDate::currentDate();
}

// Delegue applique a toutes les colonnes : une livraison en retard colore toute la ligne
// (fond rouge clair, texte rouge, gras). Sinon seule la colonne Etat est coloree.
class LivraisonDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
protected:
    void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(opt, index);
        const bool late = isLate(index);
        if (index.column() == 6) {
            opt->text = Livraison::libelleStatut(index.data().toString());
            opt->displayAlignment = Qt::AlignCenter;
        }
        if (late) {
            opt->backgroundBrush = QColor("#FECACA");
            opt->palette.setColor(QPalette::Text, QColor("#B91C1C"));
            opt->palette.setColor(QPalette::HighlightedText, QColor("#991B1B"));
            opt->palette.setColor(QPalette::Highlight, QColor("#FCA5A5"));
            opt->font.setBold(true);
        } else if (index.column() == 6) {
            QColor bg, fg; statusColors(index.data().toString(), bg, fg);
            opt->backgroundBrush = bg; opt->palette.setColor(QPalette::Text, fg);
        }
    }
};

// ---------- Construction ----------
GestionLivraison::GestionLivraison(QWidget *parent) : QWidget(parent)
{
    setObjectName("GestionLivraison");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(R"(
      QWidget#GestionLivraison {background:#F3F8FE;color:#15365E;font-family:"Segoe UI",Arial;font-size:10pt;}
      QLabel#pageTitle {color:#103A70;font-size:23pt;font-weight:800;}
      QLabel#subtitle {color:#52749B;font-size:10pt;}
      QFrame#card {background:white;border:1px solid #D5E6FA;border-radius:11px;}
      QLabel#cardTitle {color:#123B70;font-size:11pt;font-weight:700;background:transparent;}
      QLabel#fieldLabel {color:#244D7A;font-size:9pt;font-weight:600;background:transparent;}
      QLineEdit,QComboBox,QDateEdit,QSpinBox {background:white;border:1px solid #C7DDF8;border-radius:6px;padding:6px 8px;color:#15365E;min-height:22px;}
      QLineEdit:focus,QComboBox:focus,QDateEdit:focus,QSpinBox:focus {border:1px solid #2878F0;}
      QTableView {background:white;alternate-background-color:#F7FBFF;border:1px solid #D7E7F8;border-radius:7px;gridline-color:#E5EEF8;selection-background-color:#DBEAFE;selection-color:#123B70;font-size:9pt;}
      QHeaderView::section {background:#EAF4FF;color:#163B69;border:0;border-bottom:1px solid #D7E7F8;padding:8px 4px;font-weight:700;}
      QProgressBar {background:#E5EDF7;border:0;border-radius:5px;text-align:center;height:10px;}
      QProgressBar::chunk {background:#F59E0B;border-radius:5px;}
    )");
    auto *main = new QVBoxLayout(this); main->setContentsMargins(22,18,22,18); main->setSpacing(12);

    auto *header = new QHBoxLayout;
    auto *titleCol = new QVBoxLayout;
    auto *title = new QLabel("🚚  Gestion des Livraisons"); title->setObjectName("pageTitle");
    auto *sub = new QLabel("Suivez vos livraisons en temps réel et anticipez les retards."); sub->setObjectName("subtitle");
    titleCol->addWidget(title); titleCol->addWidget(sub); header->addLayout(titleCol); header->addStretch();
    alertLabel = new QLabel;
    alertLabel->setStyleSheet("background:#EAF4FF;border:1px solid #9CC7FF;border-radius:8px;padding:11px;color:#174B8C;font-weight:600;");
    alertLabel->setMinimumWidth(310); alertLabel->setAlignment(Qt::AlignCenter);
    header->addWidget(alertLabel); main->addLayout(header);

    auto *dbRow = new QHBoxLayout;
    dbStatus = new QLabel; dbStatus->setWordWrap(true);
    initButton = actionButton("⚙  Créer les tables", "#2878F0"); initButton->hide();
    connect(initButton, &QPushButton::clicked, this, &GestionLivraison::onInitBase);
    dbRow->addWidget(dbStatus, 1); dbRow->addWidget(initButton);
    main->addLayout(dbRow);

    main->addWidget(makeForm());

    auto *lower = new QGridLayout; lower->setSpacing(12);
    lower->addWidget(makeDeliveryList(),0,0,2,1);
    lower->addWidget(makeStatistics(),0,1);
    lower->addWidget(makeDetails(),0,2);
    lower->addWidget(makeAiCard(),1,1,1,2);
    lower->setColumnStretch(0,54); lower->setColumnStretch(1,25); lower->setColumnStretch(2,25);
    lower->setRowStretch(0,48); lower->setRowStretch(1,52);
    main->addLayout(lower,1);

    loadClients();
    refresh();          // Read : affichage au demarrage
}

QWidget *GestionLivraison::makeForm()
{
    auto *outer = cardFrame(); auto *v = new QVBoxLayout(outer); v->setContentsMargins(14,12,14,12); v->setSpacing(10);
    auto *grid = new QGridLayout; grid->setContentsMargins(0,0,0,0); grid->setHorizontalSpacing(16); grid->setVerticalSpacing(8);
    for (int c = 0; c < 3; ++c) grid->setColumnStretch(c, 1);
    auto add = [&](int r, int c, const QString &name, QWidget *input) {
        auto *box = new QWidget; auto *col = new QVBoxLayout(box); col->setContentsMargins(0,0,0,0); col->setSpacing(4);
        col->addWidget(fieldLabel(name)); col->addWidget(input); grid->addWidget(box, r, c);
    };
    idEdit = new QLineEdit;
    idEdit->setValidator(new QIntValidator(1, 999999999, idEdit));
    idEdit->setPlaceholderText("Automatique si vide");
    clientCombo = new QComboBox;
    qtySpin = new QSpinBox; qtySpin->setRange(1, 1000000); qtySpin->setValue(100); qtySpin->setGroupSeparatorShown(true);
    addressEdit = new QLineEdit; addressEdit->setMaxLength(200); addressEdit->setPlaceholderText("Ex : Zone industrielle, Ben Arous");
    vehicleCombo = new QComboBox; vehicleCombo->setEditable(true);
    vehicleCombo->addItems({"Camion C-01","Camion C-02","Camionnette V-03"}); vehicleCombo->lineEdit()->setMaxLength(50);
    dateEdit = new QDateEdit(QDate::currentDate()); dateEdit->setCalendarPopup(true); dateEdit->setDisplayFormat("dd/MM/yyyy");
    stateCombo = new QComboBox;
    for (const QString &c : Livraison::statutsCodes()) stateCombo->addItem(Livraison::libelleStatut(c), c);
    add(0,0,"ID livraison",idEdit); add(0,1,"Client",clientCombo); add(0,2,"Quantité",qtySpin);
    add(1,0,"Adresse de livraison",addressEdit); add(1,1,"Véhicule",vehicleCombo); add(1,2,"Date prévue",dateEdit);
    add(2,0,"État",stateCombo);

    feedback = new QLabel; feedback->setWordWrap(true); feedback->setMinimumHeight(20);
    grid->addWidget(feedback, 2, 1, 1, 2, Qt::AlignVCenter);

    auto *actions = new QHBoxLayout; actions->setSpacing(8); actions->addStretch();
    auto *bAdd = actionButton("＋  Ajouter","#2878F0");
    auto *bMod = actionButton("✎  Modifier","#D99A0B");
    auto *bDel = actionButton("🗑  Supprimer","#EF4444");
    auto *bClr = actionButton("↺  Effacer","#6B8BB3");
    auto *bExp = actionButton("▦  Exporter Excel","#16A34A");
    connect(bAdd,&QPushButton::clicked,this,&GestionLivraison::onAjouter);
    connect(bMod,&QPushButton::clicked,this,&GestionLivraison::onModifier);
    connect(bDel,&QPushButton::clicked,this,&GestionLivraison::onSupprimer);
    connect(bClr,&QPushButton::clicked,this,&GestionLivraison::onEffacer);
    connect(bExp,&QPushButton::clicked,this,&GestionLivraison::onExporter);
    for (auto *b : {bAdd,bMod,bDel,bClr,bExp}) actions->addWidget(b);
    v->addLayout(grid); v->addLayout(actions); return outer;
}

QWidget *GestionLivraison::makeDeliveryList()
{
    auto *outer = cardFrame(); auto *v = new QVBoxLayout(outer); v->setContentsMargins(12,12,12,12); v->setSpacing(9);
    v->addWidget(cardTitle("☷  Liste des livraisons"));
    auto *tools = new QHBoxLayout; tools->setSpacing(7);
    searchEdit = new QLineEdit; searchEdit->setPlaceholderText("Rechercher ID, client, adresse, véhicule…");
    sortCombo = new QComboBox;
    sortCombo->addItem("Date prévue","date"); sortCombo->addItem("Client","client");
    sortCombo->addItem("ID livraison","id"); sortCombo->addItem("Quantité","quantite");
    filterCombo = new QComboBox; filterCombo->addItem("Tous", QString());
    for (const QString &c : Livraison::statutsCodes()) filterCombo->addItem(Livraison::libelleStatut(c), c);
    tools->addWidget(searchEdit,3); tools->addWidget(new QLabel("Trier :")); tools->addWidget(sortCombo,2);
    tools->addWidget(new QLabel("État :")); tools->addWidget(filterCombo,2); v->addLayout(tools);

    table = new QTableView;
    table->setAlternatingRowColors(true); table->setShowGrid(false); table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows); table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); table->setWordWrap(false); table->setMinimumHeight(260);
    table->setItemDelegate(new LivraisonDelegate(table));
    v->addWidget(table,1);

    connect(searchEdit,&QLineEdit::textChanged,this,&GestionLivraison::refresh);
    connect(sortCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,&GestionLivraison::refresh);
    connect(filterCombo,qOverload<int>(&QComboBox::currentIndexChanged),this,&GestionLivraison::refresh);

    auto *foot = new QHBoxLayout; countLabel = new QLabel; countLabel->setStyleSheet("color:#6383A8;");
    foot->addWidget(countLabel); foot->addStretch(); v->addLayout(foot);
    return outer;
}

QWidget *GestionLivraison::makeStatistics()
{
    auto *outer = cardFrame(); auto *v = new QVBoxLayout(outer); v->setContentsMargins(12,12,12,12); v->setSpacing(8);
    v->addWidget(cardTitle("Répartition statistique des livraisons"));
    auto *body = new QWidget; auto *h = new QHBoxLayout(body); h->setContentsMargins(0,0,0,0); h->setSpacing(8);
    donut = new LivraisonDonutWidget; h->addWidget(donut,1);
    auto *legend = new QVBoxLayout;
    const QStringList names = {"Livrées","En cours","En retard","Planifiées"};
    const QStringList colors = {"#10B981","#F59E0B","#EF4444","#2878F0"};
    for (int i = 0; i < 4; ++i) {
        auto *row = new QHBoxLayout; auto *a = new QLabel("● " + names[i]); a->setStyleSheet("color:" + colors[i] + ";");
        legendValues[i] = new QLabel("0"); legendValues[i]->setStyleSheet("color:#52749B;");
        row->addWidget(a); row->addStretch(); row->addWidget(legendValues[i]); legend->addLayout(row);
    }
    legend->addStretch(); h->addLayout(legend,1); v->addWidget(body,1); return outer;
}

QWidget *GestionLivraison::makeDetails()
{
    auto *outer = cardFrame(); auto *v = new QVBoxLayout(outer); v->setContentsMargins(12,12,12,12); v->setSpacing(9);
    v->addWidget(cardTitle("Détail de la livraison"));
    auto *head = new QHBoxLayout; detailTitle = new QLabel("Aucune sélection"); head->addWidget(detailTitle); head->addStretch();
    detailBadge = new QLabel; head->addWidget(detailBadge); v->addLayout(head);
    const QString steps[] = {"Planifiée","Chargée à l'usine","En route","Livrée"};
    for (int i = 0; i < 4; ++i) { detailSteps[i] = new QLabel("○  " + steps[i]); v->addWidget(detailSteps[i]); }
    detailInfo = new QLabel; detailInfo->setWordWrap(true); detailInfo->setStyleSheet("color:#52749B;"); v->addWidget(detailInfo);
    v->addStretch(); return outer;
}

QWidget *GestionLivraison::makeAiCard()
{
    auto *outer = cardFrame(); auto *v = new QVBoxLayout(outer); v->setContentsMargins(12,12,12,12); v->setSpacing(7);
    v->addWidget(cardTitle("♧  Retard & tournée (indicateurs)"));
    auto *r2 = new QHBoxLayout; r2->addWidget(new QLabel("Score de risque :"));
    riskBar = new QProgressBar; riskBar->setRange(0,100); riskBar->setValue(0); riskBar->setTextVisible(false); riskBar->setMaximumWidth(210);
    riskScore = new QLabel("— / 100"); r2->addWidget(riskBar); r2->addWidget(riskScore); r2->addStretch(); v->addLayout(r2);
    auto *r3 = new QHBoxLayout; r3->addWidget(new QLabel("Niveau de risque :"));
    riskLevel = new QLabel("—"); riskLevel->setAlignment(Qt::AlignCenter); riskLevel->setMinimumWidth(100); r3->addWidget(riskLevel); r3->addStretch(); v->addLayout(r3);
    auto *r4 = new QHBoxLayout; r4->addWidget(new QLabel("Départ conseillé :"));
    departLabel = new QLabel("—"); departLabel->setStyleSheet("background:#DBEAFE;color:#1D4ED8;border-radius:7px;padding:5px 12px;font-weight:700;");
    r4->addWidget(departLabel); r4->addStretch(); v->addLayout(r4);
    aiInfo = new QLabel("Sélectionnez une livraison dans la liste pour évaluer son risque de retard.");
    aiInfo->setWordWrap(true); aiInfo->setStyleSheet("color:#52749B;"); v->addWidget(aiInfo);
    v->addStretch(); return outer;
}

// ---------- Utilitaires ----------
QString GestionLivraison::cell(int row, int col) const
{
    return m_model ? m_model->data(m_model->index(row, col)).toString() : QString();
}

int GestionLivraison::currentRow() const
{
    if (!table || !table->selectionModel()) return -1;
    const auto rows = table->selectionModel()->selectedRows();
    return rows.isEmpty() ? -1 : rows.first().row();
}

void GestionLivraison::showMessage(const QString &text, bool ok)
{
    feedback->setStyleSheet(QString("color:%1;font-weight:600;").arg(ok ? "#047857" : "#B91C1C"));
    feedback->setText(text);
}

void GestionLivraison::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_model) {                  // page deja construite : actualiser clients et liste
        loadClients();
        refresh();
    }
}

void GestionLivraison::loadClients()
{
    const QVariant previous = clientCombo->currentData();
    clientCombo->clear();
    const auto list = Livraison::clients();
    for (const auto &c : list) clientCombo->addItem(c.second, c.first);
    if (list.isEmpty()) clientCombo->addItem("(aucun client en base)", 0);
    const int keep = clientCombo->findData(previous);
    if (keep >= 0) clientCombo->setCurrentIndex(keep);
}

bool GestionLivraison::readForm(Livraison &l) const
{
    l.setIdClient(clientCombo->currentData().toInt());
    l.setQuantite(qtySpin->value());
    l.setAdresse(addressEdit->text());
    l.setDate(dateEdit->date());
    l.setVehicule(vehicleCombo->currentText());
    l.setStatut(stateCombo->currentData().toString());
    return true;
}

void GestionLivraison::fillForm(int row)
{
    m_selectedId = cell(row,0).toInt();
    idEdit->setText(QString::number(m_selectedId));
    const int idx = clientCombo->findData(cell(row,7).toInt());
    if (idx >= 0) clientCombo->setCurrentIndex(idx);
    qtySpin->setValue(cell(row,2).toInt());
    addressEdit->setText(cell(row,3));
    dateEdit->setDate(QDate::fromString(cell(row,4), "dd/MM/yyyy"));
    vehicleCombo->setCurrentText(cell(row,5));
    const int si = stateCombo->findData(cell(row,6));
    if (si >= 0) stateCombo->setCurrentIndex(si);
}

// ---------- Slots CRUD (aucune requete SQL ici) ----------
void GestionLivraison::updateDbStatus(bool tablesOk)
{
    Connection &cx = Connection::createInstance();
    QString txt; bool good = false;
    if (!cx.isOpen())
        txt = "✖ Base non connectée (DSN « " + cx.dsn() + " ») : " + cx.lastErrorText();
    else if (!tablesOk)
        txt = "✖ Connecté, mais tables CLIENT / LIVRAISON inaccessibles : " + Livraison::erreurStatique();
    else { txt = "✔ Connecté à la base (DSN « " + cx.dsn() + " »)"; good = true; }
    dbStatus->setText(txt);
    dbStatus->setStyleSheet(good ? "color:#047857;font-weight:600;" : "color:#B91C1C;font-weight:600;");
    initButton->setVisible(cx.isOpen() && !tablesOk);
}

void GestionLivraison::onInitBase()
{
    if (Livraison::creerTables()) { showMessage("Tables créées / vérifiées.", true); loadClients(); }
    else showMessage("Création impossible : " + Livraison::erreurStatique(), false);
    refresh();
}

void GestionLivraison::refresh()
{
    Connection &cx = Connection::createInstance();
    if (!cx.isOpen()) cx.createConnection();          // nouvelle tentative
    const bool tablesOk = cx.isOpen() && Livraison::tablesExistent();
    updateDbStatus(tablesOk);
    if (tablesOk && clientCombo->currentData().toInt() == 0) loadClients();
    const int keepId = m_selectedId;
    QSqlQueryModel *old = m_model;
    m_model = m_tmp.afficher(searchEdit->text(), filterCombo->currentData().toString(),
                             sortCombo->currentData().toString());
    table->setModel(m_model);
    table->hideColumn(7);
    delete old;
    connect(table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &GestionLivraison::onSelectionChanged);

    countLabel->setText(QString("%1 livraison(s) affichée(s)").arg(m_model->rowCount()));

    // Reselectionne la livraison courante si elle est toujours affichee.
    for (int r = 0; r < m_model->rowCount() && keepId > 0; ++r)
        if (cell(r,0).toInt() == keepId) { table->selectRow(r); break; }
    updateStatistics();
    updateDetails();
}

void GestionLivraison::onAjouter()
{
    Livraison l; readForm(l);
    l.setId(idEdit->text().trimmed().toInt());      // 0 (champ vide) = ID automatique
    if (l.ajouter()) {
        showMessage("Livraison ajoutée avec succès.", true);
        onEffacer(); refresh();
    } else {
        showMessage(l.erreur(), false);
    }
}

void GestionLivraison::onModifier()
{
    if (m_selectedId <= 0) { showMessage("Sélectionnez d'abord une livraison dans la liste.", false); return; }
    Livraison l; readForm(l); l.setId(m_selectedId);
    if (l.modifier()) { showMessage("Livraison modifiée avec succès.", true); refresh(); }
    else showMessage(l.erreur(), false);
}

void GestionLivraison::onSupprimer()
{
    if (m_selectedId <= 0) { showMessage("Sélectionnez d'abord une livraison dans la liste.", false); return; }
    if (QMessageBox::question(this, "Supprimer", QString("Supprimer la livraison n° %1 ?").arg(m_selectedId))
            != QMessageBox::Yes) return;
    if (m_tmp.supprimer(m_selectedId)) { showMessage("Livraison supprimée.", true); onEffacer(); refresh(); }
    else showMessage(m_tmp.erreur(), false);
}

void GestionLivraison::onEffacer()
{
    m_selectedId = 0;
    if (table && table->selectionModel()) table->clearSelection();
    idEdit->clear();
    qtySpin->setValue(100); addressEdit->clear(); dateEdit->setDate(QDate::currentDate());
    stateCombo->setCurrentIndex(0);
    updateDetails();
}

void GestionLivraison::onSelectionChanged()
{
    const int r = currentRow();
    if (r >= 0) fillForm(r);
    updateDetails();
}

// ---------- Affichages derives ----------
void GestionLivraison::updateStatistics()
{
    const QMap<QString,int> s = Livraison::statistiques();
    const int vals[4] = {s["LIVREE"], s["EN_COURS"], s["EN_RETARD"], s["PLANIFIEE"]};
    int total = 0; for (int v : vals) total += v;
    QVector<int> vec; for (int i = 0; i < 4; ++i) {
        vec << vals[i];
        legendValues[i]->setText(QString("%1 (%2%)").arg(vals[i]).arg(total ? qRound(100.0*vals[i]/total) : 0));
    }
    donut->setData(vec);
    alertLabel->setText(QString("♧  %1 en retard   •   %2 en cours   •   %3 à venir").arg(vals[2]).arg(vals[1]).arg(vals[3]));
}

void GestionLivraison::updateDetails()
{
    const int r = currentRow();
    if (r < 0) {
        detailTitle->setText("Aucune sélection"); detailBadge->clear(); detailInfo->clear();
        const QString steps[] = {"Planifiée","Chargée à l'usine","En route","Livrée"};
        for (int i = 0; i < 4; ++i) detailSteps[i]->setText("○  " + steps[i]);
        riskBar->setValue(0); riskScore->setText("— / 100"); riskLevel->setText("—"); riskLevel->setStyleSheet("");
        departLabel->setText("—");
        aiInfo->setText("Sélectionnez une livraison dans la liste pour évaluer son risque de retard.");
        return;
    }
    const QString code = cell(r,6), client = cell(r,1);
    detailTitle->setText(QString("L-%1 - %2").arg(cell(r,0).toInt(),3,10,QChar('0')).arg(client));
    QColor bg, fg; statusColors(code, bg, fg);
    detailBadge->setText(Livraison::libelleStatut(code));
    detailBadge->setStyleSheet(QString("background:%1;color:%2;border-radius:10px;padding:4px 9px;font-weight:700;").arg(bg.name(), fg.name()));
    int reached = 0;                       // etapes atteintes d'apres le statut
    if (code == "LIVREE") reached = 4; else if (code == "EN_COURS") reached = 3;
    else if (code == "EN_RETARD") reached = 2; else reached = 1;
    const QString steps[] = {"Planifiée","Chargée à l'usine","En route","Livrée"};
    for (int i = 0; i < 4; ++i) detailSteps[i]->setText((i < reached ? "✓  " : "○  ") + steps[i]);
    detailInfo->setText(QString("%1 unité(s) • %2 • %3 • prévue le %4").arg(cell(r,2), cell(r,5), cell(r,3), cell(r,4)));

    // Indicateur de risque simple (regle metier, pas un modele predictif).
    int score = 25;
    if (code == "LIVREE") score = 0; else if (code == "EN_RETARD") score = 85; else if (code == "EN_COURS") score = 40;
    else {
        const int jours = QDate::currentDate().daysTo(QDate::fromString(cell(r,4), "dd/MM/yyyy"));
        if (jours <= 0) score = 60; else if (jours == 1) score = 40;
    }
    if (code != "LIVREE" && cell(r,2).toInt() > 2000) score += 10;
    score = qMin(score, 100);
    riskBar->setValue(score); riskScore->setText(QString("%1 / 100").arg(score));
    const QString lvl = score < 34 ? "Faible" : (score < 67 ? "Moyen" : "Élevé");
    const QString lc = score < 34 ? "background:#D1FAE5;color:#047857;" : (score < 67 ? "background:#FEF3C7;color:#B45309;" : "background:#FEE2E2;color:#B91C1C;");
    riskLevel->setText(lvl); riskLevel->setStyleSheet(lc + "border-radius:7px;padding:5px;font-weight:700;");
    const QTime depart = QTime(8,0).addSecs(-60 * (30 + score / 2));
    departLabel->setText(depart.toString("HH:mm") + " (avant 08:00)");
    aiInfo->setText("Estimation indicative basée sur l'état, la date prévue et la quantité.");
}

// ---------- Export ----------
void GestionLivraison::onExporter()
{
    if (!m_model) return;
    QString path = QFileDialog::getSaveFileName(this,"Exporter les livraisons","livraisons.csv","CSV compatible Excel (*.csv)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".csv", Qt::CaseInsensitive)) path += ".csv";
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) { showMessage("Impossible de créer le fichier sélectionné.", false); return; }
    QTextStream out(&file); out.setEncoding(QStringConverter::Utf8); out.setGenerateByteOrderMark(true);
    out << "ID;Client;Quantité;Adresse;Date prévue;Véhicule;État\n";
    for (int r = 0; r < m_model->rowCount(); ++r) {
        for (int c = 0; c < 7; ++c) {
            QString x = (c == 6) ? Livraison::libelleStatut(cell(r,c)) : cell(r,c);
            x.replace("\"", "\"\"");
            out << "\"" << x << "\"" << (c < 6 ? ";" : "");
        }
        out << "\n";
    }
    file.close();
    showMessage("Export terminé : " + path, true);
}
