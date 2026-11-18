#include "connection.h"
#include <QSqlQuery>

// Ctor : initialise l'attribut db avec le driver ODBC.
// La meme instance de QSqlDatabase sera utilisee partout dans l'application.
Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor : ferme la connexion si elle est encore ouverte
Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

// Meyers Singleton : instance statique locale, creee au premier appel
// (thread-safe depuis C++11), detruite automatiquement a la fin du programme.
Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    db.setDatabaseName("tasnim");   // nom de la source de donnees ODBC
    db.setUserName("tasnim");               // utilisateur Oracle
    db.setPassword("tasnim2006");              // mot de passe

    if (db.open()) {
        qDebug() << "Connexion etablie";
        return true;
    }

    qDebug() << "Echec de la connexion :" << db.lastError().text();
    return false;
}

// Verifie que la connexion est vivante avant chaque operation SQL ;
// si elle est fermee ou coupee, on la rouvre (toujours la meme instance QSqlDatabase).
bool Connection::ensureOpen()
{
    if (db.isOpen()) {
        QSqlQuery ping(db);
        if (ping.exec("SELECT 1 FROM DUAL"))
            return true;
        db.close();
    }
    return createConnection();
}
