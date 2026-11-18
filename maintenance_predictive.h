#ifndef MAINTENANCE_PREDICTIVE_H
#define MAINTENANCE_PREDICTIVE_H

#include <string>
#include <vector>

// Maintenance prédictive : analyse l'historique des pannes et des maintenances
// d'un équipement pour calculer un score de risque (0-100) et proposer une date
// de maintenance préventive. Module indépendant de Qt (dates = numéro de jour
// julien, c'est-à-dire QDate::toJulianDay()).
namespace Predictif {

enum Etat { Operationnel = 0, EnMaintenance = 1, EnPanne = 2 };
enum Niveau { Faible = 0, Moyen = 1, Eleve = 2 };

struct Entree
{
    int aujourdhui = 0;              // jour julien de la date du jour
    Etat etat = Operationnel;
    int utilisation = 0;             // niveau d'utilisation, 0..100 %
    int jourInstallation = 0;        // 0 = inconnu
    int jourDerniereMaint = 0;       // champ "dernière maintenance" (0 = inconnu)
    int jourProchaineMaint = 0;      // champ "prochaine maintenance" (0 = inconnu)
    std::vector<int> pannes;         // historique : jours des pannes
    std::vector<int> maintenances;   // historique : jours des maintenances
};

struct Resultat
{
    int score = 0;                   // 0..100
    Niveau niveau = Faible;
    int jourMaintenance = 0;         // date proposée pour la maintenance préventive
    int jourPanneEstimee = 0;        // prochaine panne estimée (0 = pas assez de données)
    int mtbfJours = 0;               // temps moyen entre pannes (0 = inconnu)
    int intervalleMaintJours = 0;    // intervalle de maintenance retenu
    int pannes12Mois = 0;            // nombre de pannes sur les 12 derniers mois
    std::vector<std::string> motifs; // explications (UTF-8), les plus importantes d'abord
};

Resultat analyser(const Entree &entree);

} // namespace Predictif

#endif // MAINTENANCE_PREDICTIVE_H
