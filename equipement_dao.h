#ifndef EQUIPEMENT_DAO_H
#define EQUIPEMENT_DAO_H

#include <QDate>
#include <QList>
#include <QString>

// Un équipement (une ligne de la table EQUIPEMENT)
struct Equipement
{
    qlonglong id = 0;
    QString designation;
    QString type;
    QString marque;
    QString modele;
    QString localisation;
    qlonglong idUsine = 0;      // 0 = non renseigné (NULL en base)
    QString etat;
    int utilisation = 0;
    QDate install;
    QDate derniere;
    QDate prochaine;
};

// Un événement de l'historique d'un équipement (table HISTORIQUE_EQUIPEMENT).
// type vaut "Panne" ou "Maintenance".
struct Evenement
{
    qlonglong id = 0;
    qlonglong idEquipement = 0;
    QString type;
    QDate date;
    QString description;
};

// Accès base de données : toutes les requêtes SQL du module sont ici.
// Les noms de table/colonnes sont définis en haut de equipement_dao.cpp.
class EquipementDao
{
public:
    static bool lister(QList<Equipement> &resultat, QString &erreur);
    static bool ajouter(const Equipement &e, QString &erreur, qlonglong *idCree = nullptr);
    static bool modifier(const Equipement &e, QString &erreur);
    static bool supprimer(qlonglong id, QString &erreur);

    // Historique des pannes et maintenances (nécessite creation_historique.sql)
    static bool listerEvenements(QList<Evenement> &resultat, QString &erreur,
                                 qlonglong idEquipement = -1);
    static bool ajouterEvenement(const Evenement &ev, QString &erreur, bool eviterDoublon = false);
    static bool supprimerEvenement(qlonglong id, QString &erreur);
};

#endif // EQUIPEMENT_DAO_H
