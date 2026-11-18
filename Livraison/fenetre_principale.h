#ifndef FENETRE_PRINCIPALE_H
#define FENETRE_PRINCIPALE_H

#include <QMainWindow>
#include <QVector>

class QStackedWidget;
class QPushButton;

// Fenetre principale : menu lateral (Accueil, Employes, Produits, Equipements, Usines,
// Clients, Livraisons, Statistiques, Exportation, Parametres) + pages empilees.
// Seul le module Livraisons est realise ; les autres pages sont des pages vides.
class FenetrePrincipale : public QMainWindow {
    Q_OBJECT
public:
    explicit FenetrePrincipale(QWidget *parent = nullptr);

private slots:
    void afficherPage(int index);

private:
    QWidget *creerMenu();
    QWidget *creerPageVide(const QString &titre);
    QPushButton *creerBouton(const QString &texte, int index);

    QStackedWidget *m_pages = nullptr;
    QVector<QPushButton *> m_boutons;
};

#endif // FENETRE_PRINCIPALE_H
