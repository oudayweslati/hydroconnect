#ifndef LIVRAISON_H
#define LIVRAISON_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QMap>
#include <QVector>
#include <QPair>

class QSqlQueryModel;

// Entite "Livraison" (classe purement C++/Qt, couche Modele).
// MCD : Livraison(id_livraison, quantite, adresse_livraison, date_livraison,
//                 vehicule, statut) ; un Client recoit 0..N livraisons et
//                 chaque livraison concerne exactement un client (id_client).
// Toutes les requetes SQL du module sont ici (requetes preparees uniquement).
class Livraison
{
public:
    // Codes de statut stockes en base (sans accents) : PLANIFIEE, EN_COURS, LIVREE, EN_RETARD
    Livraison();
    Livraison(int id, int idClient, int quantite, const QString &adresse,
              const QDate &date, const QString &vehicule, const QString &statut);

    // Getters
    int id() const { return m_id; }
    int idClient() const { return m_idClient; }
    int quantite() const { return m_quantite; }
    QString adresse() const { return m_adresse; }
    QDate date() const { return m_date; }
    QString vehicule() const { return m_vehicule; }
    QString statut() const { return m_statut; }

    // Setters
    void setId(int v) { m_id = v; }
    void setIdClient(int v) { m_idClient = v; }
    void setQuantite(int v) { m_quantite = v; }
    void setAdresse(const QString &v) { m_adresse = v; }
    void setDate(const QDate &v) { m_date = v; }
    void setVehicule(const QString &v) { m_vehicule = v; }
    void setStatut(const QString &v) { m_statut = v; }

    // Controle de saisie : renvoie "" si valide, sinon le message d'erreur.
    QString valider() const;

    // CRUD
    // Create : si id() > 0 il est utilise tel quel (erreur s'il existe deja),
    // sinon l'id est genere par la sequence SEQ_LIVRAISON.
    bool ajouter();
    // Vrai si une livraison portant cet identifiant existe deja en base.
    static bool existe(int id);
    bool modifier();                // Update (selon id())
    bool supprimer(int id);         // Delete
    // Read : modele pret a afficher dans un QTableView.
    // Colonnes : 0 ID_LIVRAISON, 1 CLIENT, 2 QUANTITE, 3 ADRESSE_LIVRAISON,
    //            4 DATE_LIVRAISON (JJ/MM/AAAA), 5 VEHICULE, 6 STATUT, 7 ID_CLIENT
    // tri : "date" | "client" | "id" | "quantite" ; statut : "" (tous) ou un code.
    QSqlQueryModel *afficher(const QString &recherche = QString(),
                             const QString &statut = QString(),
                             const QString &tri = QStringLiteral("date"));

    // Statistiques : nombre de livraisons par code de statut.
    static QMap<QString, int> statistiques();
    // Clients disponibles (id, nom) pour la cle etrangere.
    static QVector<QPair<int, QString>> clients();

    // Verifie que la table LIVRAISON est accessible ; creerTables() cree
    // CLIENT / LIVRAISON / SEQ_LIVRAISON si absentes (et 3 clients d'exemple si CLIENT est vide).
    static bool tablesExistent();
    static bool creerTables();

    static QStringList statutsCodes();
    static QString libelleStatut(const QString &code);
    // Erreur de la derniere operation faite SUR CET OBJET (ajouter, modifier, supprimer, afficher).
    QString erreur() const { return m_erreur; }
    // Erreur de la derniere operation statique (clients, statistiques, tablesExistent, creerTables).
    static QString erreurStatique() { return s_erreur; }

private:
    int m_id = 0;
    int m_idClient = 0;
    int m_quantite = 0;
    QString m_adresse;
    QDate m_date;
    QString m_vehicule;
    QString m_statut;
    QString m_erreur;               // propre a chaque objet
    static QString s_erreur;        // uniquement pour les methodes statiques
};

#endif // LIVRAISON_H
