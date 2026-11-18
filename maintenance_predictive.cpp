#include "maintenance_predictive.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace Predictif {
namespace {

int moyenneIntervalles(std::vector<int> jours)
{
    std::sort(jours.begin(), jours.end());
    jours.erase(std::unique(jours.begin(), jours.end()), jours.end());
    if (jours.size() < 2)
        return 0;
    return int(std::lround(double(jours.back() - jours.front()) / double(jours.size() - 1)));
}

// Le jour julien 0 est un lundi : (jour % 7) = 0 lundi ... 5 samedi, 6 dimanche.
int prochainJourOuvre(int jour)
{
    while (jour % 7 >= 5)
        ++jour;
    return jour;
}

} // namespace

Resultat analyser(const Entree &in)
{
    Resultat r;
    std::vector<std::pair<double, std::string>> motifs;   // (poids, texte)

    // ---------------------------------------------------------------- pannes
    double pondere = 0;
    for (int jour : in.pannes) {
        const int age = in.aujourdhui - jour;
        if (age >= 0 && age <= 365) {
            ++r.pannes12Mois;
            pondere += std::exp(-age / 180.0);        // une panne récente pèse plus
        }
    }
    const double ptsPannes = std::min(35.0, pondere * 14.0);
    if (r.pannes12Mois > 0) {
        motifs.push_back({ptsPannes, std::to_string(r.pannes12Mois)
                                         + (r.pannes12Mois > 1 ? " pannes" : " panne")
                                         + " sur les 12 derniers mois"});
    } else {
        motifs.push_back({0.5, "Aucune panne enregistrée sur les 12 derniers mois"});
    }

    // MTBF et prochaine panne estimée (au moins 2 pannes pour calculer un intervalle)
    r.mtbfJours = moyenneIntervalles(in.pannes);
    if (r.mtbfJours > 0 && !in.pannes.empty()) {
        const int derniere = *std::max_element(in.pannes.begin(), in.pannes.end());
        r.jourPanneEstimee = derniere + r.mtbfJours;
        if (r.jourPanneEstimee < in.aujourdhui)
            r.jourPanneEstimee = in.aujourdhui;       // déjà « en retard » sur son cycle
        motifs.push_back({1.0, "Temps moyen entre pannes : " + std::to_string(r.mtbfJours) + " jours"});
    }

    // ------------------------------------------------------- maintenance
    int intervalle = moyenneIntervalles(in.maintenances);
    if (intervalle <= 0)
        intervalle = 180;                              // valeur par défaut : 6 mois
    intervalle = std::max(30, std::min(365, intervalle));
    r.intervalleMaintJours = intervalle;

    int derniere = in.jourDerniereMaint;
    for (int jour : in.maintenances)
        derniere = std::max(derniere, jour);
    if (derniere > in.aujourdhui)
        derniere = in.aujourdhui;                      // date future : ignorée

    double ptsRetard = 0;
    if (derniere > 0) {
        const int depuis = in.aujourdhui - derniere;
        const double ratio = double(depuis) / double(intervalle);
        ptsRetard = std::max(0.0, std::min(30.0, (ratio - 0.6) * 40.0));
        if (ratio >= 1.0) {
            motifs.push_back({ptsRetard, "Dernière maintenance il y a " + std::to_string(depuis)
                                             + " jours (intervalle habituel : "
                                             + std::to_string(intervalle) + " jours)"});
        } else if (ptsRetard > 0) {
            motifs.push_back({ptsRetard, "Dernière maintenance il y a " + std::to_string(depuis) + " jours"});
        }
    } else {
        ptsRetard = 10;
        motifs.push_back({ptsRetard, "Aucune maintenance enregistrée"});
    }

    // ----------------------------------------------------- utilisation / état
    const int util = std::max(0, std::min(100, in.utilisation));
    const double ptsUtil = util * 0.15;
    if (util >= 70)
        motifs.push_back({ptsUtil, "Niveau d'utilisation élevé (" + std::to_string(util) + " %)"});

    double ptsEtat = 0;
    if (in.etat == EnPanne) {
        ptsEtat = 15;
        motifs.push_back({ptsEtat, "Équipement actuellement en panne"});
    } else if (in.etat == EnMaintenance) {
        ptsEtat = 5;
        motifs.push_back({ptsEtat, "Équipement actuellement en maintenance"});
    }

    double ptsPlanning = 0;
    if (in.jourProchaineMaint > 0 && in.jourProchaineMaint < in.aujourdhui) {
        ptsPlanning = 5;
        motifs.push_back({ptsPlanning, "Maintenance planifiée dépassée de "
                                           + std::to_string(in.aujourdhui - in.jourProchaineMaint) + " jours"});
    }

    // ------------------------------------------------------------- score
    const double total = ptsPannes + ptsRetard + ptsUtil + ptsEtat + ptsPlanning;
    r.score = std::max(0, std::min(100, int(std::lround(total))));
    r.niveau = r.score < 35 ? Faible : (r.score < 65 ? Moyen : Eleve);

    // ------------------------------------- date de maintenance préventive
    int jour;
    if (in.etat == EnPanne) {
        jour = in.aujourdhui;
        motifs.push_back({99, "Intervention immédiate recommandée"});
    } else {
        const double facteur = 1.0 - 0.5 * r.score / 100.0;       // plus de risque = plus tôt
        const int base = derniere > 0 ? derniere : in.aujourdhui;
        jour = base + int(std::lround(intervalle * facteur));

        if (r.jourPanneEstimee > 0) {
            const int marge = std::max(7, r.mtbfJours / 10);
            jour = std::min(jour, r.jourPanneEstimee - marge);   // avant la panne prévue
        }

        const int delaiMin = r.score >= 65 ? 2 : (r.score >= 35 ? 7 : 14);
        const int plusTot = in.aujourdhui + delaiMin;
        // On respecte la date déjà planifiée si elle est plus proche que la date calculée.
        if (in.jourProchaineMaint >= plusTot && in.jourProchaineMaint < jour)
            jour = in.jourProchaineMaint;
        if (jour < plusTot)
            jour = plusTot;
        jour = prochainJourOuvre(jour);
    }
    r.jourMaintenance = jour;

    std::stable_sort(motifs.begin(), motifs.end(),
                     [](const std::pair<double, std::string> &a, const std::pair<double, std::string> &b) {
                         return a.first > b.first;
                     });
    for (const auto &m : motifs)
        r.motifs.push_back(m.second);
    return r;
}

} // namespace Predictif
