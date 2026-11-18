#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>

// Singleton : une seule connexion Oracle (ODBC) pour toute l'application.
// L'API reste celle de votre projet : Connection::createConnection()
class Connection
{
public:
    static Connection& createInstance();     // instance unique (Meyers Singleton)
    static bool createConnection();          // ouvre la connexion Oracle

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

private:
    Connection();
    ~Connection();

    bool ouvrir();
    QSqlDatabase db;
};

#endif // CONNECTION_H
