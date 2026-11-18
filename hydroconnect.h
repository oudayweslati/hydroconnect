#ifndef HYDROCONNECT_H
#define HYDROCONNECT_H

#include <QMainWindow>
#include <QList>
#include "usine.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class QTextBrowser;
class QStackedWidget;

class HydroConnect : public QMainWindow
{
    Q_OBJECT

public:
    explicit HydroConnect(QWidget *parent = nullptr);

private slots:
    void rafraichir();
    void ajouterUsine();
    void modifierUsine();
    void supprimerUsine();
    void ligneSelectionnee();
    void exporterExcel();
    void exporterPdf();
    void simuler();
    void questionIA();
    void reponseRapide(const QString &cle);

private:
    QWidget *creerMenu();
    QWidget *creerPageUsines();
    QWidget *creerCarte(const QString &tag, const QString &bg, const QString &fg,
                        const QString &titre, QLabel *&valeur, const QString &sousTitre);
    Usine usineDepuisFormulaire() const;
    QString etatCouleur(const QString &etat) const;
    QString reponseIA(const QString &cle) const;

    QList<Usine> m_usines;     // liste affichee (apres filtres)
    QList<Usine> m_toutes;     // toutes les usines

    QStackedWidget *m_stack = nullptr;
    QLabel *m_lblNb = nullptr, *m_lblOp = nullptr, *m_lblMaint = nullptr, *m_lblDebit = nullptr;
    QLineEdit *m_recherche = nullptr;
    QComboBox *m_tri = nullptr, *m_filtreEtat = nullptr;
    QTableWidget *m_table = nullptr;

    // Formulaire
    QSpinBox *m_fId = nullptr;
    QLineEdit *m_fNom = nullptr, *m_fLoc = nullptr;
    QComboBox *m_fType = nullptr, *m_fEtat = nullptr;
    QDoubleSpinBox *m_fCap = nullptr, *m_fDebit = nullptr, *m_fConso = nullptr;

    // Simulateur / IA
    QDoubleSpinBox *m_sDebit = nullptr, *m_sHeures = nullptr, *m_sKwh = nullptr, *m_sPrix = nullptr;
    QLabel *m_sResultat = nullptr;
    QTextBrowser *m_chat = nullptr;
    QLineEdit *m_question = nullptr;
};

#endif // HYDROCONNECT_H
