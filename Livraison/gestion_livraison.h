#ifndef GESTION_LIVRAISON_H
#define GESTION_LIVRAISON_H

#include <QWidget>
#include "livraison.h"

class QTableView;
class QSqlQueryModel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDateEdit;
class QLabel;
class QProgressBar;
class QPushButton;
class LivraisonDonutWidget;
class QItemSelection;
class QShowEvent;

// Vue du module Livraisons : aucune requete SQL ici, tout passe par la classe Livraison.
class GestionLivraison : public QWidget {
    Q_OBJECT
public:
    explicit GestionLivraison(QWidget *parent = nullptr);

protected:
    void showEvent(QShowEvent *event) override;   // recharge les clients a chaque affichage

private slots:
    void onAjouter();
    void onModifier();
    void onSupprimer();
    void onEffacer();
    void onExporter();
    void refresh();
    void onSelectionChanged();
    void onInitBase();

private:
    QWidget *makeForm();
    QWidget *makeDeliveryList();
    QWidget *makeStatistics();
    QWidget *makeDetails();
    QWidget *makeAiCard();

    bool readForm(Livraison &l) const;
    void fillForm(int row);
    void loadClients();
    void updateDbStatus(bool tablesOk);
    void updateStatistics();
    void updateDetails();
    void showMessage(const QString &text, bool ok);
    int currentRow() const;
    QString cell(int row, int col) const;

    Livraison m_tmp;                       // objet Livraison utilise pour les appels CRUD
    QSqlQueryModel *m_model = nullptr;
    int m_selectedId = 0;

    QTableView *table = nullptr;
    QLineEdit *searchEdit = nullptr;
    QComboBox *sortCombo = nullptr;
    QComboBox *filterCombo = nullptr;
    QLineEdit *idEdit = nullptr;
    QComboBox *clientCombo = nullptr;
    QSpinBox *qtySpin = nullptr;
    QLineEdit *addressEdit = nullptr;
    QComboBox *vehicleCombo = nullptr;
    QDateEdit *dateEdit = nullptr;
    QComboBox *stateCombo = nullptr;
    QLabel *feedback = nullptr;
    QLabel *countLabel = nullptr;
    QLabel *alertLabel = nullptr;
    QLabel *dbStatus = nullptr;
    QPushButton *initButton = nullptr;
    LivraisonDonutWidget *donut = nullptr;
    QLabel *legendValues[4] = {nullptr, nullptr, nullptr, nullptr};
    QLabel *detailTitle = nullptr;
    QLabel *detailBadge = nullptr;
    QLabel *detailSteps[4] = {nullptr, nullptr, nullptr, nullptr};
    QLabel *detailInfo = nullptr;
    QProgressBar *riskBar = nullptr;
    QLabel *riskScore = nullptr;
    QLabel *riskLevel = nullptr;
    QLabel *departLabel = nullptr;
    QLabel *aiInfo = nullptr;
};

#endif // GESTION_LIVRAISON_H
