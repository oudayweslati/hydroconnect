#include "usine.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// ============================================================
// Table et colonnes Oracle (d'apres votre schema TASNIM_FINAL)
// Si une colonne change de nom, modifiez UNIQUEMENT ces lignes.
// ============================================================
#define T_USINE   "USINE"
#define C_ID      "ID_USINE"
#define C_NOM     "NOM"
#define C_LOC     "LOCALISATION"
#define C_TYPE    "TYPE_TRAITEMENT"
#define C_CAP     "CAPACITE"
#define C_DEBIT   "DEBIT_ACTUEL"
#define C_CONSO   "CONSOMMATION"
#define C_ETAT    "ETAT_OPERATIONNEL"

// Votre table contient aussi les colonnes NOM_USINE et ETAT.
// Avec EXTRA_COLONNES = 1, elles recoivent une copie du nom et de l'etat
// (utile si elles sont obligatoires). Mettez 0 pour les ignorer.
#define EXTRA_COLONNES 1
#define C_NOM2    "NOM_USINE"
#define C_ETAT2   "ETAT"

static QString s_erreur;

static bool executer(QSqlQuery &q)
{
    s_erreur.clear();
    if (!q.exec()) {
        s_erreur = q.lastError().text();
        qWarning() << "Erreur SQL :" << s_erreur;
        return false;
    }
    return true;
}

QString Usine::derniereErreur()
{
    return s_erreur;
}

Usine::Usine(int id, const QString &nom, const QString &localisation, const QString &type,
             double capacite, double debit, double conso, const QString &etat)
    : id(id), nom(nom), localisation(localisation), type(type),
      capacite(capacite), debit(debit), conso(conso), etat(etat)
{
}

bool Usine::ajouter() const
{
    QString cols = QString("INSERT INTO " T_USINE " (" C_ID ", " C_NOM ", " C_LOC ", " C_TYPE ", "
                           C_CAP ", " C_DEBIT ", " C_CONSO ", " C_ETAT);
    QString vals = ":id, :nom, :loc, :type, :cap, :deb, :conso, :etat";
#if EXTRA_COLONNES
    cols += ", " C_NOM2 ", " C_ETAT2;
    vals += ", :nom2, :etat2";
#endif

    QSqlQuery q;
    q.prepare(cols + ") VALUES (" + vals + ")");
    q.bindValue(":id", id);
    q.bindValue(":nom", nom);
    q.bindValue(":loc", localisation);
    q.bindValue(":type", type);
    q.bindValue(":cap", capacite);
    q.bindValue(":deb", debit);
    q.bindValue(":conso", conso);
    q.bindValue(":etat", etat);
#if EXTRA_COLONNES
    q.bindValue(":nom2", nom);
    q.bindValue(":etat2", etat);
#endif
    return executer(q);
}

bool Usine::modifier() const
{
    QString sets = QString(C_NOM "=:nom, " C_LOC "=:loc, " C_TYPE "=:type, "
                           C_CAP "=:cap, " C_DEBIT "=:deb, " C_CONSO "=:conso, " C_ETAT "=:etat");
#if EXTRA_COLONNES
    sets += ", " C_NOM2 "=:nom2, " C_ETAT2 "=:etat2";
#endif

    QSqlQuery q;
    q.prepare("UPDATE " T_USINE " SET " + sets + " WHERE " C_ID "=:id");
    q.bindValue(":nom", nom);
    q.bindValue(":loc", localisation);
    q.bindValue(":type", type);
    q.bindValue(":cap", capacite);
    q.bindValue(":deb", debit);
    q.bindValue(":conso", conso);
    q.bindValue(":etat", etat);
#if EXTRA_COLONNES
    q.bindValue(":nom2", nom);
    q.bindValue(":etat2", etat);
#endif
    q.bindValue(":id", id);
    return executer(q) && q.numRowsAffected() > 0;
}

bool Usine::supprimer(int id)
{
    QSqlQuery q;
    q.prepare("DELETE FROM " T_USINE " WHERE " C_ID "=:id");
    q.bindValue(":id", id);
    return executer(q) && q.numRowsAffected() > 0;
}

QList<Usine> Usine::tous()
{
    QList<Usine> liste;
    QSqlQuery q;
    q.prepare("SELECT " C_ID ", " C_NOM ", " C_LOC ", " C_TYPE ", " C_CAP ", "
              C_DEBIT ", " C_CONSO ", " C_ETAT " FROM " T_USINE " ORDER BY " C_ID);
    if (!executer(q))
        return liste;

    while (q.next()) {
        liste.append(Usine(q.value(0).toInt(), q.value(1).toString(), q.value(2).toString(),
                           q.value(3).toString(), q.value(4).toDouble(), q.value(5).toDouble(),
                           q.value(6).toDouble(), q.value(7).toString()));
    }
    return liste;
}

int Usine::prochainId()
{
    QSqlQuery q;
    q.prepare("SELECT NVL(MAX(" C_ID "), 100) + 1 FROM " T_USINE);
    return (executer(q) && q.next()) ? q.value(0).toInt() : 101;
}

bool Usine::existe(int id)
{
    QSqlQuery q;
    q.prepare("SELECT 1 FROM " T_USINE " WHERE " C_ID "=:id");
    q.bindValue(":id", id);
    return executer(q) && q.next();
}
