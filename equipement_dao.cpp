#include "equipement_dao.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

// ====== Noms de la table et des colonnes (à adapter si votre table diffère) ======
#define TABLE        "EQUIPEMENT"
#define C_ID         "ID_EQUIPEMENT"
#define C_DESIGN     "NOM"
#define C_TYPE       "TYPE"
#define C_MARQUE     "MARQUE"
#define C_MODELE     "MODELE"
#define C_LOCAL      "LOCALISATION"
#define C_INSTALL    "DATE_INSTALLATION"
#define C_ETAT       "ETAT"
#define C_UTIL       "NIVEAU_UTILISATION"
#define C_DERNIERE   "DERNIERE_MAINTENANCE"
#define C_PROCHAINE  "PROCHAINE_MAINTENANCE"
#define C_USINE      "ID_USINE"
// =================================================================================

// ====== Table d'historique (créée par creation_historique.sql) ======
#define T_HIST       "HISTORIQUE_EQUIPEMENT"
#define H_ID         "ID_HISTORIQUE"
#define H_EQUIP      "ID_EQUIPEMENT"
#define H_TYPE       "TYPE_EVENEMENT"
#define H_DATE       "DATE_EVENEMENT"
#define H_DESC       "DESCRIPTION"
// =====================================================================

// ====== Valeurs de ETAT acceptées par la contrainte CHK_ETAT ======
//   SELECT search_condition FROM user_constraints WHERE constraint_name = 'CHK_ETAT';
#define BASE_OPERATIONNEL  "Operationnel"
#define BASE_MAINTENANCE   "En maintenance"
#define BASE_PANNE         "En panne"
// =================================================================

namespace {
const char *kFmt = "dd/MM/yyyy";

// Libellé de l'interface -> valeur acceptée par la contrainte CHK_ETAT
QString etatVersBase(const QString &etat)
{
    if (etat == "Opérationnel")   return BASE_OPERATIONNEL;
    if (etat == "En maintenance") return BASE_MAINTENANCE;
    if (etat == "En panne")       return BASE_PANNE;
    return etat;
}

// Valeur de la base -> libellé affiché dans l'interface
QString etatVersAffichage(const QString &etat)
{
    if (etat == BASE_OPERATIONNEL) return "Opérationnel";
    if (etat == BASE_MAINTENANCE)  return "En maintenance";
    if (etat == BASE_PANNE)        return "En panne";
    return etat;
}

// Oracle : TO_CHAR / TO_DATE. SQLite (tests locaux) : dates stockées en texte.
bool estOracle()
{
    return QSqlDatabase::database().driverName() != "QSQLITE";
}

QString lireDate(const char *colonne)
{
    return estOracle() ? QString("TO_CHAR(%1,'DD/MM/YYYY')").arg(colonne) : QString(colonne);
}

QString ecrireDate()
{
    return estOracle() ? "TO_DATE(?,'DD/MM/YYYY')" : "?";
}

QString texteDate(const QDate &d)
{
    return d.isValid() ? d.toString(kFmt) : QString();
}

// ID usine écrit directement dans la requête (nombre) : NULL si non renseigné.
// (le pilote Oracle ODBC gère mal les paramètres entiers 64 bits)
QString usineSql(const Equipement &e)
{
    return e.idUsine > 0 ? QString::number(e.idUsine) : QString("NULL");
}

void lier(QSqlQuery &q, const Equipement &e)
{
    q.addBindValue(e.designation);
    q.addBindValue(e.type);
    q.addBindValue(e.marque);
    q.addBindValue(e.modele);
    q.addBindValue(e.localisation);
    q.addBindValue(texteDate(e.install));
    q.addBindValue(etatVersBase(e.etat));
    q.addBindValue(e.utilisation);
    q.addBindValue(texteDate(e.derniere));
    q.addBindValue(texteDate(e.prochaine));
}

bool executer(QSqlQuery &q, QString &erreur)
{
    if (!q.exec()) {
        erreur = q.lastError().text();
        return false;
    }
    return true;
}

// Pour UPDATE / DELETE : échoue aussi si aucune ligne n'a été touchée.
bool executerModif(QSqlQuery &q, qlonglong id, QString &erreur)
{
    if (!q.exec()) {
        erreur = q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0) {
        erreur = QString("Aucune ligne trouvée pour l'identifiant %1.").arg(id);
        return false;
    }
    return true;
}

// Prochain identifiant libre (MAX + 1) de la table donnée.
bool prochainId(const char *table, const char *colonne, qlonglong &id, QString &erreur)
{
    QSqlQuery q;
    if (!q.exec(QString("SELECT COALESCE(MAX(%1), 0) + 1 FROM %2").arg(colonne, table))) {
        erreur = q.lastError().text();
        return false;
    }
    id = q.next() ? q.value(0).toLongLong() : 1;
    return true;
}
} // namespace

bool EquipementDao::lister(QList<Equipement> &resultat, QString &erreur)
{
    resultat.clear();
    if (!QSqlDatabase::database().isOpen()) {
        erreur = "La base de données n'est pas connectée.";
        return false;
    }

    QSqlQuery q;
    const QString sql = QString(
                            "SELECT " C_ID ", " C_DESIGN ", " C_TYPE ", " C_MARQUE ", " C_MODELE ", " C_LOCAL ", %1, "
                            C_ETAT ", " C_UTIL ", %2, %3, " C_USINE " FROM " TABLE " ORDER BY " C_ID)
                            .arg(lireDate(C_INSTALL), lireDate(C_DERNIERE), lireDate(C_PROCHAINE));

    if (!q.exec(sql)) {
        erreur = q.lastError().text();
        return false;
    }

    while (q.next()) {
        Equipement e;
        e.id           = q.value(0).toLongLong();
        e.designation  = q.value(1).toString();
        e.type         = q.value(2).toString();
        e.marque       = q.value(3).toString();
        e.modele       = q.value(4).toString();
        e.localisation = q.value(5).toString();
        e.install      = QDate::fromString(q.value(6).toString(), kFmt);
        e.etat         = etatVersAffichage(q.value(7).toString());
        e.utilisation  = q.value(8).toInt();
        e.derniere     = QDate::fromString(q.value(9).toString(), kFmt);
        e.prochaine    = QDate::fromString(q.value(10).toString(), kFmt);
        e.idUsine      = q.value(11).isNull() ? 0 : q.value(11).toLongLong();
        resultat.append(e);
    }
    return true;
}

bool EquipementDao::ajouter(const Equipement &e, QString &erreur, qlonglong *idCree)
{
    qlonglong id = 0;
    if (!prochainId(TABLE, C_ID, id, erreur))
        return false;

    // L'identifiant est écrit directement dans la requête (c'est un nombre calculé
    // par nous) : le pilote Oracle ODBC gère mal les paramètres de type entier 64 bits.
    QSqlQuery q;
    q.prepare(QString(
                  "INSERT INTO " TABLE " (" C_ID ", " C_DESIGN ", " C_TYPE ", " C_MARQUE ", " C_MODELE ", "
                  C_LOCAL ", " C_INSTALL ", " C_ETAT ", " C_UTIL ", " C_DERNIERE ", " C_PROCHAINE ", " C_USINE ") "
                  "VALUES (%1, ?, ?, ?, ?, ?, %2, ?, ?, %2, %2, %3)")
                  .arg(QString::number(id), ecrireDate(), usineSql(e)));
    lier(q, e);
    if (!executer(q, erreur))
        return false;
    if (idCree)
        *idCree = id;
    return true;
}

bool EquipementDao::modifier(const Equipement &e, QString &erreur)
{
    QSqlQuery q;
    q.prepare(QString(
                  "UPDATE " TABLE " SET " C_DESIGN "=?, " C_TYPE "=?, " C_MARQUE "=?, " C_MODELE "=?, "
                  C_LOCAL "=?, " C_INSTALL "=%1, " C_ETAT "=?, " C_UTIL "=?, " C_DERNIERE "=%1, "
                  C_PROCHAINE "=%1, " C_USINE "=%3 WHERE " C_ID "=%2")
                  .arg(ecrireDate(), QString::number(e.id), usineSql(e)));
    lier(q, e);
    return executerModif(q, e.id, erreur);
}

bool EquipementDao::supprimer(qlonglong id, QString &erreur)
{
    QSqlQuery q;
    q.prepare(QString("DELETE FROM " TABLE " WHERE " C_ID "=%1").arg(QString::number(id)));
    if (!executerModif(q, id, erreur))
        return false;

    // Nettoyage de l'historique de cet équipement (sans importance s'il n'existe pas) :
    // évite qu'un futur équipement réutilisant ce numéro hérite de ses anciennes pannes.
    QSqlQuery h;
    h.exec(QString("DELETE FROM " T_HIST " WHERE " H_EQUIP "=%1").arg(QString::number(id)));
    return true;
}

// ------------------------------------------------------------------ historique
bool EquipementDao::listerEvenements(QList<Evenement> &resultat, QString &erreur,
                                     qlonglong idEquipement)
{
    resultat.clear();
    if (!QSqlDatabase::database().isOpen()) {
        erreur = "La base de données n'est pas connectée.";
        return false;
    }

    QString sql = QString("SELECT " H_ID ", " H_EQUIP ", " H_TYPE ", %1, " H_DESC " FROM " T_HIST)
                      .arg(lireDate(H_DATE));
    if (idEquipement >= 0)
        sql += QString(" WHERE " H_EQUIP "=%1").arg(QString::number(idEquipement));
    sql += " ORDER BY " H_DATE " DESC, " H_ID " DESC";

    QSqlQuery q;
    if (!q.exec(sql)) {
        erreur = q.lastError().text();
        return false;
    }
    while (q.next()) {
        Evenement ev;
        ev.id           = q.value(0).toLongLong();
        ev.idEquipement = q.value(1).toLongLong();
        ev.type         = q.value(2).toString();
        ev.date         = QDate::fromString(q.value(3).toString(), kFmt);
        ev.description  = q.value(4).toString();
        resultat.append(ev);
    }
    return true;
}

bool EquipementDao::ajouterEvenement(const Evenement &ev, QString &erreur, bool eviterDoublon)
{
    if (!ev.date.isValid()) {
        erreur = "Date invalide.";
        return false;
    }

    if (eviterDoublon) {
        QSqlQuery c;
        c.prepare(QString("SELECT COUNT(*) FROM " T_HIST " WHERE " H_EQUIP "=%1 AND " H_TYPE "=? AND "
                          H_DATE "=%2").arg(QString::number(ev.idEquipement), ecrireDate()));
        c.addBindValue(ev.type);
        c.addBindValue(texteDate(ev.date));
        if (!c.exec()) {
            erreur = c.lastError().text();
            return false;
        }
        if (c.next() && c.value(0).toInt() > 0)
            return true;                    // déjà enregistré : rien à faire
    }

    qlonglong id = 0;
    if (!prochainId(T_HIST, H_ID, id, erreur))
        return false;

    QSqlQuery q;
    q.prepare(QString("INSERT INTO " T_HIST " (" H_ID ", " H_EQUIP ", " H_TYPE ", " H_DATE ", " H_DESC ") "
                      "VALUES (%1, %2, ?, %3, ?)")
                  .arg(QString::number(id), QString::number(ev.idEquipement), ecrireDate()));
    q.addBindValue(ev.type);
    q.addBindValue(texteDate(ev.date));
    q.addBindValue(ev.description.left(200));
    return executer(q, erreur);
}

bool EquipementDao::supprimerEvenement(qlonglong id, QString &erreur)
{
    QSqlQuery q;
    q.prepare(QString("DELETE FROM " T_HIST " WHERE " H_ID "=%1").arg(QString::number(id)));
    return executerModif(q, id, erreur);
}
