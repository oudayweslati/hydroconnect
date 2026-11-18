#ifndef STATISTIQUES_DIALOG_H
#define STATISTIQUES_DIALOG_H

#include <QDate>
#include <QDialog>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

#include "equipement_dao.h"

// Chiffres affichés dans la fenêtre « Statistiques » et exportés dans Excel.
struct StatsEquipements
{
    int total = 0;
    int operationnels = 0;
    int enMaintenance = 0;
    int enPanne = 0;
    double disponibilite = 0;                       // % d'équipements opérationnels

    bool historiqueDisponible = false;              // table HISTORIQUE_EQUIPEMENT lisible ?
    QString messageHistorique;                      // explication si indisponible
    QStringList moisLabels;                         // 12 derniers mois (du plus ancien au plus récent)
    QList<int> pannesParMois;                       // nombre de pannes pour chaque mois
    int totalPannes12Mois = 0;
    double moyennePannesParMois = 0;
    QList<QPair<QString, int>> pannesParEquipement; // 5 équipements les plus touchés (12 mois)
    QList<QPair<QString, int>> parType;             // nombre d'équipements par type
};

StatsEquipements calculerStats(const QList<Equipement> &equipements,
                               const QList<Evenement> &evenements,
                               bool historiqueDisponible,
                               const QString &messageHistorique,
                               const QDate &aujourdhui = QDate::currentDate());

// Fenêtre avec les graphiques : états, taux de disponibilité, fréquence des pannes.
class StatistiquesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatistiquesDialog(const StatsEquipements &stats, QWidget *parent = nullptr);
};

#endif // STATISTIQUES_DIALOG_H
