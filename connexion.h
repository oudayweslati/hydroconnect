#ifndef CONNEXION_H
#define CONNEXION_H

#include <QString>

// Connexion Oracle (via ODBC) utilisée par toute l'application.
// Elle est ouverte comme connexion PAR DÉFAUT de Qt, ce qui permet au DAO
// d'utiliser simplement QSqlDatabase::database() / QSqlQuery q;
class Connexion
{
public:
    static bool ouvrir(QString &erreur);
    static void fermer();
};

#endif // CONNEXION_H
