#include "connection.h"

#include <QSqlError>
#include <QDebug>

Connection::Connection()
{
    // Connexion par defaut (sans nom) : les QSqlQuery de la classe Usine
    // l'utilisent automatiquement.
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    return createInstance().ouvrir();
}

bool Connection::ouvrir()
{
    if (db.isOpen())
        return true;

    db.setDatabaseName("tasnim");   // <-- nom de VOTRE source ODBC Oracle
    db.setUserName("tasnim");
    db.setPassword("tasnim2006");

    if (!db.open()) {
        qWarning() << "Connexion Oracle impossible :" << db.lastError().text();
        return false;
    }

    qDebug() << "Connexion Oracle etablie";
    return true;
}
