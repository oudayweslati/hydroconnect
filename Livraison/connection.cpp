#include "connection.h"
#include <QString>
#include <QtGlobal>

// Ctor
// Initialise l'attribut `db` avec le driver ODBC (connexion par defaut de Qt).
// La meme instance de QSqlDatabase est utilisee partout dans l'application
// via le Singleton, au lieu de creer une nouvelle connexion a chaque fois.
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

// Dtor
// Ferme la connexion si elle est encore ouverte lorsque l'objet est detruit
Connection::~Connection() {
    if (db.isOpen())
        db.close(); // Libere proprement les ressources
}

// --- Acces a l'instance unique du Singleton ---
// Technique "Meyers Singleton" : instance statique locale, creee au premier appel
// de createInstance() et vivante pendant toute la duree du programme (thread-safe en C++11+)
Connection& Connection::createInstance() {
    static Connection instance;  // Instance unique du Singleton
    return instance;             // Retourne une reference a cette instance
}

bool Connection::createConnection() {
    if (db.isOpen())
        return true;

    // Parametres : variables d'environnement si definies, sinon valeurs par defaut de l'atelier.
    auto param = [](const char *nom, const QString &defaut) {
        const QString v = qEnvironmentVariable(nom);
        return v.isEmpty() ? defaut : v;
    };
    db.setDatabaseName(param("HYDROCONNECT_DB_DSN",      QStringLiteral("tasnim")));      // DSN ODBC
    db.setUserName(    param("HYDROCONNECT_DB_USER",     QStringLiteral("tasnim")));      // utilisateur
    db.setPassword(    param("HYDROCONNECT_DB_PASSWORD", QStringLiteral("tasnim2006")));  // mot de passe

    if (db.open()) {
        m_erreur.clear();
        qDebug() << "Connexion établie";
        return true;
    }

    m_erreur = db.lastError().text();
    qDebug() << "Échec de la connexion :" << m_erreur;
    return false;
}
