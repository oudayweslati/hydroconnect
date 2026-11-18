#ifndef SCHEMA_H
#define SCHEMA_H

#include <QString>

// Schema minimal du module Livraison : CLIENT (cle etrangere) et LIVRAISON + sequences.
class SchemaBD
{
public:
    static bool tableExiste(const QString &nom);
    static bool complet();          // toutes les tables existent ?
    static bool creer();            // cree ce qui manque (+ clients d'exemple si CLIENT est vide)
    static bool assurer();          // creer() seulement si le schema est incomplet
    static QString derniereErreur() { return s_erreur; }
private:
    static QString s_erreur;
};

#endif // SCHEMA_H
