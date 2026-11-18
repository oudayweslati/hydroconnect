#ifndef USINE_H
#define USINE_H

#include <QString>
#include <QList>

class Usine
{
public:
    Usine() = default;
    Usine(int id, const QString &nom, const QString &localisation, const QString &type,
          double capacite, double debit, double conso, const QString &etat);

    // CRUD (requetes preparees, table Oracle USINE)
    bool ajouter() const;
    bool modifier() const;
    static bool supprimer(int id);
    static QList<Usine> tous();
    static int prochainId();
    static bool existe(int id);

    // Dernier message d'erreur Oracle (pour les QMessageBox)
    static QString derniereErreur();

    int     id = 0;
    QString nom;
    QString localisation;
    QString type;
    double  capacite = 0;   // m3
    double  debit = 0;      // m3/h
    double  conso = 0;      // kWh
    QString etat;
};

#endif // USINE_H
