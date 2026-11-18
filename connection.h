#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

/*
 * Classe Connection
 * Patron Singleton (Meyers Singleton) : une seule connexion a la base Oracle
 * pour toute l'application, via le driver ODBC (QODBC).
 * Acces : Connection::createInstance()
 */
class Connection
{
public:
    /****** Methodes publiques ******/
    static Connection& createInstance();   // retourne la reference vers l'instance unique
    bool createConnection();               // configure et ouvre la connexion a la base
    QSqlDatabase database() const { return db; }
    bool ensureOpen();                     // verifie la connexion avant chaque operation (la rouvre si besoin)
    QString lastErrorText() const { return db.lastError().text(); }

    // Interdire la copie et l'affectation : garantit l'unicite de l'instance
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

private:
    QSqlDatabase db;                       // connexion persistante (vit autant que le singleton)

    Connection();                          // constructeur prive
    ~Connection();                         // destructeur prive : ferme la connexion
};

#endif // CONNECTION_H
