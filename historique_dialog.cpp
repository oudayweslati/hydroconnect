#include "historique_dialog.h"

#include <QAbstractItemView>
#include <QColor>
#include <QComboBox>
#include <QDateEdit>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

namespace {
const char *kFmt = "dd/MM/yyyy";

const char *kStyleHist = R"CSS(
    QDialog { background:#F6F9FF; }
    QWidget { color:#0B2A55; font-family:'Segoe UI'; font-size:12px; }
    QLabel { background:transparent; }
    QLabel#titre { font-size:16px; font-weight:800; }
    QLabel#sousTitre { color:#6C7C98; }
    QLabel#erreur { color:#B26A00; background:#FFF4E0; border-radius:8px; padding:8px 10px; }
    QLineEdit, QComboBox, QDateEdit {
        background:white; border:1px solid #D5E1F5; border-radius:9px;
        padding:3px 10px; min-height:22px; color:#0B2A55;
    }
    QComboBox QAbstractItemView { background:white; color:#0B2A55;
        selection-background-color:#DCE9FF; selection-color:#0B2A55; }
    QTableWidget { background:white; alternate-background-color:#F5F9FF; border:1px solid #E3EBF7;
        border-radius:10px; selection-background-color:#DCE9FF; selection-color:#0B2A55; gridline-color:transparent; }
    QHeaderView::section { background:#E8F0FD; color:#0B2A55; border:none; padding:6px; font-weight:800; }
    QPushButton { border:none; border-radius:9px; padding:0 16px; min-height:32px; font-weight:800; }
    QPushButton#add { background:#2F6FED; color:white; }
    QPushButton#add:hover { background:#2461D6; }
    QPushButton#del { background:white; color:#0B2A55; border:1px solid #D5E1F5; }
    QPushButton#del:hover { background:#F4F8FF; }
    QPushButton#close { background:#EAF2FF; color:#2F6FED; }
)CSS";
} // namespace

HistoriqueDialog::HistoriqueDialog(qlonglong idEquipement, const QString &nomEquipement, QWidget *parent)
    : QDialog(parent), m_id(idEquipement)
{
    setWindowTitle("Historique des pannes et maintenances");
    resize(640, 520);
    setStyleSheet(kStyleHist);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(10);

    auto *titre = new QLabel(QString("Historique : %1 - %2").arg(idEquipement).arg(nomEquipement));
    titre->setObjectName("titre");
    auto *sous = new QLabel("Ces événements servent à la maintenance prédictive (score de risque, date conseillée) "
                            "et aux statistiques de pannes.");
    sous->setObjectName("sousTitre");
    sous->setWordWrap(true);
    root->addWidget(titre);
    root->addWidget(sous);

    m_message = new QLabel;
    m_message->setObjectName("erreur");
    m_message->setWordWrap(true);
    m_message->hide();
    root->addWidget(m_message);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"Date", "Type", "Description"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 110);
    m_table->setColumnWidth(1, 120);
    root->addWidget(m_table, 1);

    m_type = new QComboBox;
    m_type->addItems({"Panne", "Maintenance"});
    m_date = new QDateEdit(QDate::currentDate());
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat(kFmt);
    m_description = new QLineEdit;
    m_description->setPlaceholderText("Ex : roulement usé, remplacement du joint...");
    m_description->setMaxLength(200);

    auto *form = new QHBoxLayout;
    form->setSpacing(8);
    form->addWidget(m_type, 1);
    form->addWidget(m_date, 1);
    form->addWidget(m_description, 3);
    root->addLayout(form);

    m_ajouter = new QPushButton("Ajouter l'événement");
    m_ajouter->setObjectName("add");
    m_supprimer = new QPushButton("Supprimer la sélection");
    m_supprimer->setObjectName("del");
    auto *fermer = new QPushButton("Fermer");
    fermer->setObjectName("close");
    for (auto *b : {m_ajouter, m_supprimer, fermer})
        b->setCursor(Qt::PointingHandCursor);

    auto *boutons = new QHBoxLayout;
    boutons->addWidget(m_ajouter);
    boutons->addWidget(m_supprimer);
    boutons->addStretch();
    boutons->addWidget(fermer);
    root->addLayout(boutons);

    connect(m_ajouter, &QPushButton::clicked, this, &HistoriqueDialog::onAjouter);
    connect(m_supprimer, &QPushButton::clicked, this, &HistoriqueDialog::onSupprimer);
    connect(fermer, &QPushButton::clicked, this, &QDialog::accept);

    recharger();
}

void HistoriqueDialog::recharger()
{
    QString erreur;
    const bool ok = EquipementDao::listerEvenements(m_liste, erreur, m_id);

    m_table->setRowCount(0);
    for (const Evenement &ev : m_liste) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(ev.date.isValid() ? ev.date.toString(kFmt) : QString()));
        auto *type = new QTableWidgetItem(ev.type);
        QFont f = type->font();
        f.setBold(true);
        type->setFont(f);
        type->setForeground(ev.type == "Panne" ? QColor("#E5484D") : QColor("#2F6FED"));
        m_table->setItem(row, 1, type);
        m_table->setItem(row, 2, new QTableWidgetItem(ev.description));
    }

    m_ajouter->setEnabled(ok);
    m_supprimer->setEnabled(ok);
    if (!ok) {
        m_message->setText("Historique indisponible. Exécutez le script creation_historique.sql dans "
                           "SQL Developer (connexion tasnim), puis rouvrez cette fenêtre.\n\n" + erreur);
        m_message->show();
    } else {
        m_message->hide();
    }
}

void HistoriqueDialog::onAjouter()
{
    Evenement ev;
    ev.idEquipement = m_id;
    ev.type = m_type->currentText();
    ev.date = m_date->date();
    ev.description = m_description->text().trimmed();

    if (ev.date > QDate::currentDate()) {
        QMessageBox::warning(this, "Date invalide", "La date d'un événement ne peut pas être dans le futur.");
        return;
    }

    QString erreur;
    if (!EquipementDao::ajouterEvenement(ev, erreur)) {
        QMessageBox::critical(this, "Ajout impossible", "Erreur de base de données :\n\n" + erreur);
        return;
    }
    m_modifie = true;
    m_description->clear();
    recharger();
}

void HistoriqueDialog::onSupprimer()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_liste.size() || m_table->selectedItems().isEmpty()) {
        QMessageBox::information(this, "Supprimer", "Sélectionnez un événement dans la liste.");
        return;
    }
    QString erreur;
    if (!EquipementDao::supprimerEvenement(m_liste.at(row).id, erreur)) {
        QMessageBox::critical(this, "Suppression impossible", "Erreur de base de données :\n\n" + erreur);
        return;
    }
    m_modifie = true;
    recharger();
}
