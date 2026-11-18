#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QString>

/* Classe Connection (atelier "Connexion - Singleton de Meyers")
 * Pattern Singleton : une seule connexion a la base de donnees pour toute l'application.
 * L'instance est accessible via la methode statique createInstance().
 * La methode createConnection() configure et ouvre la base de donnees (driver QODBC).
 */
class Connection {
public:

    /******Methodes publiques*****/
    /****************************/

    static Connection& createInstance(); // Retourne la reference unique
    bool createConnection();             // Etablit la connexion a la base

    // Complements utilises par les modules (acces a la connexion et diagnostic)
    QSqlDatabase database() const { return db; }
    bool isOpen() const { return db.isOpen(); }
    QString lastErrorText() const { return m_erreur; }
    QString dsn() const { return db.databaseName(); }

private:

    // Connexion persistante : attribut de l'instance unique (le Singleton)
    QSqlDatabase db;
    QString m_erreur;

    /******Methodes privees*****/
    /**************************/

    Connection();   // Constructeur prive : empeche la creation d'autres instances
    ~Connection();  // Destructeur prive : ferme proprement la connexion

    // Copie et affectation interdites : garantit l'unicite de la connexion
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H
