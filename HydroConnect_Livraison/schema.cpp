#include "schema.h"
#include "connection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>

QString SchemaBD::s_erreur;

static QSqlDatabase bd() { return Connection::createInstance().database(); }

// Les tables sont creees dans l'ordre des dependances (cles etrangeres).
static const char *const kTables[] = {
    "CREATE TABLE CLIENT ( "
        "    ID_CLIENT           NUMBER        PRIMARY KEY, "
        "    NOM_RAISON_SOCIALE  VARCHAR2(100) NOT NULL CONSTRAINT UQ_CLI_NOM UNIQUE, "
        "    EMAIL               VARCHAR2(100) NOT NULL CONSTRAINT UQ_CLI_EMAIL UNIQUE, "
        "    TELEPHONE           VARCHAR2(30)  NOT NULL, "
        "    CATEGORIE           VARCHAR2(20)  DEFAULT 'ENTREPRISE' NOT NULL "
        "        CONSTRAINT CK_CLI_CAT CHECK (CATEGORIE IN ('GROSSISTE','DETAILLANT','ENTREPRISE')), "
        "    CHIFFRE_AFFAIRES    NUMBER(14,2)  DEFAULT 0 NOT NULL CONSTRAINT CK_CLI_CA CHECK (CHIFFRE_AFFAIRES >= 0) "
        ") ",
    "CREATE TABLE LIVRAISON ( "
        "    ID_LIVRAISON       NUMBER         PRIMARY KEY, "
        "    QUANTITE           NUMBER(8)      NOT NULL CONSTRAINT CK_LIV_QTE CHECK (QUANTITE > 0), "
        "    ADRESSE_LIVRAISON  VARCHAR2(200)  NOT NULL, "
        "    DATE_LIVRAISON     DATE           NOT NULL, "
        "    VEHICULE           VARCHAR2(50)   NOT NULL, "
        "    STATUT             VARCHAR2(20)   DEFAULT 'PLANIFIEE' NOT NULL "
        "        CONSTRAINT CK_LIV_STATUT CHECK (STATUT IN ('PLANIFIEE','EN_COURS','LIVREE','EN_RETARD')), "
        "    ID_CLIENT          NUMBER         NOT NULL CONSTRAINT FK_LIV_CLIENT REFERENCES CLIENT(ID_CLIENT) "
        ") ",
};

static const char *const kSequences[] = {
    "CREATE SEQUENCE SEQ_CLIENT START WITH 1 INCREMENT BY 1 NOCACHE",
    "CREATE SEQUENCE SEQ_LIVRAISON START WITH 1 INCREMENT BY 1 NOCACHE",
};

bool SchemaBD::tableExiste(const QString &nom)
{
    QSqlQuery q(bd());
    // Le nom vient d'une liste interne fixe (jamais d'une saisie utilisateur).
    return q.exec(QStringLiteral("SELECT 1 FROM ") + nom + QStringLiteral(" WHERE 1 = 0"));
}

bool SchemaBD::complet()
{
    static const char *noms[] = {"CLIENT","LIVRAISON"};
    for (const char *n : noms)
        if (!tableExiste(QString::fromLatin1(n))) return false;
    return true;
}

bool SchemaBD::creer()
{
    s_erreur.clear();
    if (!bd().isOpen()) { s_erreur = QStringLiteral("Base non connectée."); return false; }

    for (const char *ddl : kTables) {
        QSqlQuery q(bd());
        q.exec(QString::fromUtf8(ddl));     // erreur "existe deja" ignoree volontairement
    }
    for (const char *ddl : kSequences) {
        QSqlQuery q(bd());
        q.exec(QString::fromUtf8(ddl));
    }

    // Clients d'exemple si la table est vide.
    QSqlQuery c(bd());
    if (c.exec(QStringLiteral("SELECT COUNT(*) FROM CLIENT")) && c.next() && c.value(0).toInt() == 0) {
        struct Ex { const char *nom, *email, *tel, *cat; double ca; };
        const Ex ex[] = {
            {"SONEDE", "contact@sonede.tn", "71 123 456", "ENTREPRISE", 85000},
            {"Aqua Plus", "aqua@plus.tn", "98 765 432", "GROSSISTE", 62500},
            {"Eau Pure", "eaupure@gmail.com", "22 345 678", "DETAILLANT", 35200},
            {"Hydro Tunisie", "contact@hydro.tn", "55 987 654", "ENTREPRISE", 29800},
            {"Crystal Water", "crystal@water.tn", "23 456 789", "GROSSISTE", 28400},
            {"Blue Wave", "blue@wave.tn", "96 321 654", "DETAILLANT", 22700},
            {"Fresh Water", "fresh@gmail.com", "27 654 321", "DETAILLANT", 19300},
            {"Aqua Distribution", "contact@aqua.tn", "52 963 741", "GROSSISTE", 18600},
        };
        for (const Ex &e : ex) {
            QSqlQuery ins(bd());
            ins.prepare(QStringLiteral(
                "INSERT INTO CLIENT (ID_CLIENT, NOM_RAISON_SOCIALE, EMAIL, TELEPHONE, CATEGORIE, CHIFFRE_AFFAIRES) "
                "VALUES (SEQ_CLIENT.NEXTVAL, :nom, :email, :tel, :cat, :ca)"));
            ins.bindValue(":nom", QString::fromUtf8(e.nom));
            ins.bindValue(":email", QString::fromUtf8(e.email));
            ins.bindValue(":tel", QString::fromUtf8(e.tel));
            ins.bindValue(":cat", QString::fromUtf8(e.cat));
            ins.bindValue(":ca", e.ca);
            ins.exec();
        }
    }
    if (!complet()) {
        QSqlQuery q(bd());
        q.exec(QStringLiteral("SELECT 1 FROM LIVRAISON WHERE 1 = 0"));
        s_erreur = q.lastError().text();
        return false;
    }
    return true;
}

bool SchemaBD::assurer()
{
    if (!bd().isOpen()) return false;
    if (complet()) return true;
    return creer();
}
