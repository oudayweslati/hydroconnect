#include "livraison.h"
#include "connection.h"
#include "schema.h"

#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QSqlDatabase>
#include <QRegularExpression>
#include <QVariant>
#include <utility>

QString Livraison::s_erreur;

static QSqlDatabase bd() { return Connection::createInstance().database(); }

Livraison::Livraison() : m_statut(QStringLiteral("PLANIFIEE")) {}

Livraison::Livraison(int id, int idClient, int quantite, const QString &adresse,
                     const QDate &date, const QString &vehicule, const QString &statut)
    : m_id(id), m_idClient(idClient), m_quantite(quantite), m_adresse(adresse),
      m_date(date), m_vehicule(vehicule), m_statut(statut) {}

QStringList Livraison::statutsCodes()
{
    return {QStringLiteral("PLANIFIEE"), QStringLiteral("EN_COURS"),
            QStringLiteral("LIVREE"), QStringLiteral("EN_RETARD")};
}

QString Livraison::libelleStatut(const QString &code)
{
    if (code == QLatin1String("PLANIFIEE")) return QStringLiteral("Planifiée");
    if (code == QLatin1String("EN_COURS"))  return QStringLiteral("En cours");
    if (code == QLatin1String("LIVREE"))    return QStringLiteral("Livrée");
    if (code == QLatin1String("EN_RETARD")) return QStringLiteral("En retard");
    return code;
}

QString Livraison::valider() const
{
    if (m_idClient <= 0)
        return QStringLiteral("Veuillez choisir un client.");
    if (m_quantite <= 0 || m_quantite > 1000000)
        return QStringLiteral("La quantité doit être un entier entre 1 et 1 000 000.");
    const QString adr = m_adresse.trimmed();
    if (adr.length() < 3 || adr.length() > 200)
        return QStringLiteral("L'adresse doit contenir entre 3 et 200 caractères.");
    if (!m_date.isValid())
        return QStringLiteral("La date de livraison est invalide.");
    const QString veh = m_vehicule.trimmed();
    if (veh.isEmpty() || veh.length() > 50)
        return QStringLiteral("Le véhicule est obligatoire (50 caractères maximum).");
    static const QRegularExpression rx(QStringLiteral("^[\\p{L}0-9 .\\-_/]+$"));
    if (!rx.match(veh).hasMatch())
        return QStringLiteral("Le véhicule contient des caractères non autorisés.");
    if (!statutsCodes().contains(m_statut))
        return QStringLiteral("Le statut est invalide.");
    return QString();
}

bool Livraison::existe(int id)
{
    QSqlQuery q(bd());
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM LIVRAISON WHERE ID_LIVRAISON = :id"));
    q.bindValue(":id", id);
    if (!q.exec()) { s_erreur = q.lastError().text(); return false; }
    return q.next() && q.value(0).toInt() > 0;
}

bool Livraison::ajouter()
{
    m_erreur = valider();
    if (!m_erreur.isEmpty()) return false;

    const bool idManuel = m_id > 0;
    if (idManuel && existe(m_id)) {
        m_erreur = QStringLiteral("Ajout impossible : une livraison avec l'ID %1 existe déjà.").arg(m_id);
        return false;
    }

    const QString sql = QStringLiteral(
        "INSERT INTO LIVRAISON (ID_LIVRAISON, QUANTITE, ADRESSE_LIVRAISON, DATE_LIVRAISON, "
        "VEHICULE, STATUT, ID_CLIENT) VALUES (")
        + (idManuel ? QStringLiteral(":id") : QStringLiteral("SEQ_LIVRAISON.NEXTVAL"))
        + QStringLiteral(", :quantite, :adresse, TO_DATE(:dateliv, 'YYYY-MM-DD'), "
                         ":vehicule, :statut, :idclient)");

    // Mode automatique : si la sequence rencontre un ID deja saisi a la main (ORA-00001),
    // on retente avec la valeur suivante.
    const int essais = idManuel ? 1 : 100;
    for (int n = 0; n < essais; ++n) {
        QSqlQuery q(bd());
        q.prepare(sql);
        if (idManuel) q.bindValue(":id", m_id);
        q.bindValue(":quantite", m_quantite);
        q.bindValue(":adresse", m_adresse.trimmed());
        q.bindValue(":dateliv", m_date.toString(QStringLiteral("yyyy-MM-dd")));
        q.bindValue(":vehicule", m_vehicule.trimmed());
        q.bindValue(":statut", m_statut);
        q.bindValue(":idclient", m_idClient);
        if (q.exec()) return true;
        const QString txt = q.lastError().text();
        if (txt.contains(QLatin1String("ORA-00001"))) {
            if (idManuel) {
                m_erreur = QStringLiteral("Ajout impossible : une livraison avec l'ID %1 existe déjà.").arg(m_id);
                return false;
            }
            continue;
        }
        m_erreur = txt;
        return false;
    }
    m_erreur = QStringLiteral("Impossible de générer un identifiant libre pour la livraison.");
    return false;
}

bool Livraison::modifier()
{
    m_erreur = valider();
    if (!m_erreur.isEmpty()) return false;
    if (m_id <= 0) { m_erreur = QStringLiteral("Aucune livraison sélectionnée."); return false; }

    QSqlQuery q(bd());
    q.prepare(QStringLiteral(
        "UPDATE LIVRAISON SET QUANTITE = :quantite, ADRESSE_LIVRAISON = :adresse, "
        "DATE_LIVRAISON = TO_DATE(:dateliv, 'YYYY-MM-DD'), VEHICULE = :vehicule, "
        "STATUT = :statut, ID_CLIENT = :idclient WHERE ID_LIVRAISON = :id"));
    q.bindValue(":quantite", m_quantite);
    q.bindValue(":adresse", m_adresse.trimmed());
    q.bindValue(":dateliv", m_date.toString(QStringLiteral("yyyy-MM-dd")));
    q.bindValue(":vehicule", m_vehicule.trimmed());
    q.bindValue(":statut", m_statut);
    q.bindValue(":idclient", m_idClient);
    q.bindValue(":id", m_id);
    if (!q.exec()) { m_erreur = q.lastError().text(); return false; }
    if (q.numRowsAffected() == 0) { m_erreur = QStringLiteral("Livraison introuvable."); return false; }
    return true;
}

bool Livraison::supprimer(int id)
{
    m_erreur.clear();
    QSqlQuery q(bd());
    q.prepare(QStringLiteral("DELETE FROM LIVRAISON WHERE ID_LIVRAISON = :id"));
    q.bindValue(":id", id);
    if (!q.exec()) { m_erreur = q.lastError().text(); return false; }
    if (q.numRowsAffected() == 0) { m_erreur = QStringLiteral("Livraison introuvable."); return false; }
    return true;
}

QSqlQueryModel *Livraison::afficher(const QString &recherche, const QString &statut, const QString &tri)
{
    m_erreur.clear();
    QString sql = QStringLiteral(
        "SELECT l.ID_LIVRAISON, c.NOM_RAISON_SOCIALE AS CLIENT, l.QUANTITE, l.ADRESSE_LIVRAISON, "
        "TO_CHAR(l.DATE_LIVRAISON, 'DD/MM/YYYY') AS DATE_LIVRAISON, l.VEHICULE, l.STATUT, l.ID_CLIENT "
        "FROM LIVRAISON l LEFT JOIN CLIENT c ON c.ID_CLIENT = l.ID_CLIENT WHERE 1 = 1");

    const QString r = recherche.trimmed();
    if (!r.isEmpty())
        sql += QStringLiteral(" AND (TO_CHAR(l.ID_LIVRAISON) LIKE :r1 OR UPPER(c.NOM_RAISON_SOCIALE) LIKE :r2"
                              " OR UPPER(l.ADRESSE_LIVRAISON) LIKE :r3 OR UPPER(l.VEHICULE) LIKE :r4)");
    const bool filtreStatut = statutsCodes().contains(statut);
    if (filtreStatut)
        sql += QStringLiteral(" AND l.STATUT = :statut");

    // Le tri ne peut pas etre lie par bindValue : liste blanche de valeurs fixes.
    QString order = QStringLiteral("l.DATE_LIVRAISON, l.ID_LIVRAISON");
    if (tri == QLatin1String("client"))        order = QStringLiteral("c.NOM_RAISON_SOCIALE, l.ID_LIVRAISON");
    else if (tri == QLatin1String("id"))       order = QStringLiteral("l.ID_LIVRAISON");
    else if (tri == QLatin1String("quantite")) order = QStringLiteral("l.QUANTITE DESC, l.ID_LIVRAISON");
    sql += QStringLiteral(" ORDER BY ") + order;

    QSqlQuery q(bd());
    q.prepare(sql);
    if (!r.isEmpty()) {
        const QString like = QLatin1Char('%') + r.toUpper() + QLatin1Char('%');
        q.bindValue(":r1", like); q.bindValue(":r2", like);
        q.bindValue(":r3", like); q.bindValue(":r4", like);
    }
    if (filtreStatut) q.bindValue(":statut", statut);
    q.exec();
    if (q.lastError().isValid()) m_erreur = q.lastError().text();

    auto *model = new QSqlQueryModel;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    model->setQuery(std::move(q));
#else
    model->setQuery(q);
#endif
    model->setHeaderData(0, Qt::Horizontal, QStringLiteral("ID"));
    model->setHeaderData(1, Qt::Horizontal, QStringLiteral("Client"));
    model->setHeaderData(2, Qt::Horizontal, QStringLiteral("Quantité"));
    model->setHeaderData(3, Qt::Horizontal, QStringLiteral("Adresse"));
    model->setHeaderData(4, Qt::Horizontal, QStringLiteral("Date prévue"));
    model->setHeaderData(5, Qt::Horizontal, QStringLiteral("Véhicule"));
    model->setHeaderData(6, Qt::Horizontal, QStringLiteral("État"));
    return model;
}

QMap<QString, int> Livraison::statistiques()
{
    QMap<QString, int> res;
    for (const QString &s : statutsCodes()) res[s] = 0;
    QSqlQuery q(bd());
    q.prepare(QStringLiteral("SELECT STATUT, COUNT(*) FROM LIVRAISON GROUP BY STATUT"));
    if (!q.exec()) { s_erreur = q.lastError().text(); return res; }
    while (q.next()) res[q.value(0).toString()] = q.value(1).toInt();
    return res;
}

QVector<QPair<int, QString>> Livraison::clients()
{
    QVector<QPair<int, QString>> res;
    QSqlQuery q(bd());
    q.prepare(QStringLiteral("SELECT ID_CLIENT, NOM_RAISON_SOCIALE FROM CLIENT ORDER BY NOM_RAISON_SOCIALE"));
    if (!q.exec()) { s_erreur = q.lastError().text(); return res; }
    while (q.next()) res.append({q.value(0).toInt(), q.value(1).toString()});
    return res;
}

bool Livraison::tablesExistent()
{
    QSqlQuery q(bd());
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM LIVRAISON"));
    if (!q.exec()) { s_erreur = q.lastError().text(); return false; }
    QSqlQuery c(bd());
    c.prepare(QStringLiteral("SELECT COUNT(*) FROM CLIENT"));
    if (!c.exec()) { s_erreur = c.lastError().text(); return false; }
    return true;
}

bool Livraison::creerTables()
{
    const bool ok = SchemaBD::creer();
    if (!ok) s_erreur = SchemaBD::derniereErreur();
    return ok;
}
