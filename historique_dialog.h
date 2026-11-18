#ifndef HISTORIQUE_DIALOG_H
#define HISTORIQUE_DIALOG_H

#include <QDialog>
#include <QList>

#include "equipement_dao.h"

class QComboBox;
class QDateEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

// Fenêtre : historique des pannes et maintenances d'un équipement.
// C'est cet historique qui alimente la maintenance prédictive et les statistiques.
class HistoriqueDialog : public QDialog
{
    Q_OBJECT

public:
    HistoriqueDialog(qlonglong idEquipement, const QString &nomEquipement,
                     QWidget *parent = nullptr);

    bool modifie() const { return m_modifie; }   // true si des événements ont été ajoutés/supprimés

private:
    void recharger();
    void onAjouter();
    void onSupprimer();

    qlonglong m_id;
    bool m_modifie = false;
    QList<Evenement> m_liste;

    QTableWidget *m_table = nullptr;
    QComboBox *m_type = nullptr;
    QDateEdit *m_date = nullptr;
    QLineEdit *m_description = nullptr;
    QLabel *m_message = nullptr;
    QPushButton *m_ajouter = nullptr;
    QPushButton *m_supprimer = nullptr;
};

#endif // HISTORIQUE_DIALOG_H
