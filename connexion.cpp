#include "connexion.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace {
// Nom de la source de données ODBC (à créer dans l'administrateur ODBC)
const char *kDsn  = "SmartWaterFactory";
const char *kUser = "tasnim";
const char *kPass = "tasnim2006";
}

bool Connexion::ouvrir(QString &erreur)
{
    if (!QSqlDatabase::isDriverAvailable("QODBC")) {
        erreur = "Le pilote Qt « QODBC » n'est pas disponible.";
        return false;
    }

    // Connexion par défaut (sans nom) : c'est celle que le DAO utilise.
    QSqlDatabase db;
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
        db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
    else
        db = QSqlDatabase::addDatabase("QODBC");

    if (db.isOpen())
        return true;

    db.setDatabaseName(kDsn);
    db.setUserName(kUser);
    db.setPassword(kPass);

    if (!db.open()) {
        erreur = db.lastError().text();
        qDebug() << "Erreur connexion Oracle:" << erreur;
        return false;
    }

    qDebug() << "Connexion Oracle reussie !";
    return true;
}

void Connexion::fermer()
{
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
        if (db.isOpen())
            db.close();
    }
}
