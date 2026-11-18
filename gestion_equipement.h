#ifndef GESTION_EQUIPEMENT_H
#define GESTION_EQUIPEMENT_H

#include <QList>
#include <QStringList>
#include <QWidget>

#include "equipement_dao.h"
#include "maintenance_predictive.h"

class QComboBox;
class QDateEdit;
class QFrame;
class QLabel;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTabWidget;
class QTimer;
class DonutWidget;
class MachineWidget;

class GestionEquipement : public QWidget
{
    Q_OBJECT

public:
    explicit GestionEquipement(QWidget *parent = nullptr);

private:
    enum Col {
        cID, cDesignation, cType, cMarque, cModele, cLocalisation,
        cInstall, cEtat, cUtilisation, cDerniere, cProchaine, cIdUsine, cCount
    };

    // construction de l'interface
    void appliquerTheme();
    void buildUi();
    QWidget *buildEntete();
    QFrame  *buildCarteDetails();
    QFrame  *buildCarteEtat();
    QFrame  *buildCarteTypes();
    QFrame  *buildCarteFormulaire();
    QFrame  *buildCarteListe();
    QWidget *buildOnglets();

    // données (base Oracle)
    void recharger(qlonglong idASelectionner = -1);
    void chargerEvenements();
    void signalerErreur(const QString &titre, const QString &detail);
    void ecrireLigne(int row, const QStringList &valeurs);
    QString texte(int row, int col) const;
    int  trouverLigneParID(const QString &id) const;
    Equipement equipementDepuisLigne(int row) const;
    void journaliserChangement(const Equipement *ancien, const Equipement &nouveau);

    // formulaire
    void viderFormulaire();
    void remplirFormulaire(int row);
    bool lireFormulaire(Equipement &e);

    // rafraîchissement
    void afficherDetails(int row);
    void rafraichirTout();
    void rafraichirResume();
    void rafraichirListeIA();
    void rafraichirClassementIA();
    void rafraichirAlertes();
    void appliquerFiltre();
    void majHorloge();
    void majBoutonOrdre();

    // actions
    void onSelection();
    void onAjouter();
    void onModifier();
    void onSupprimer();
    void onTri();
    void onExporter();
    bool exporterCsv(const QString &chemin, const QList<int> &lignes, QString &erreur) const;
    bool exporterXlsx(const QString &chemin, const QList<int> &lignes, QString &erreur) const;
    void onStatistiques();
    void onAnalyser();
    void onHistorique();
    void onAppliquerDate();
    Predictif::Resultat analyserLigne(int row) const;

    // en-tête
    QLabel *m_date = nullptr;
    QLabel *m_heure = nullptr;
    QTimer *m_timer = nullptr;

    // carte détails
    QLabel *m_dNom = nullptr;
    QLabel *m_dType = nullptr;
    QLabel *m_dLoc = nullptr;
    QLabel *m_dEtat = nullptr;
    MachineWidget *m_machine = nullptr;

    // carte état (donut)
    DonutWidget *m_donut = nullptr;
    QLabel *m_legOp = nullptr;
    QLabel *m_legMaint = nullptr;
    QLabel *m_legPanne = nullptr;

    // carte répartition par type
    QLabel *m_typeNom[3] = {nullptr, nullptr, nullptr};
    QLabel *m_typePct[3] = {nullptr, nullptr, nullptr};
    QProgressBar *m_typeBar[3] = {nullptr, nullptr, nullptr};

    // formulaire
    QLineEdit *m_id = nullptr;
    QLineEdit *m_designation = nullptr;
    QLineEdit *m_marque = nullptr;
    QLineEdit *m_modele = nullptr;
    QLineEdit *m_localisation = nullptr;
    QLineEdit *m_idUsine = nullptr;
    QComboBox *m_type = nullptr;
    QComboBox *m_etat = nullptr;
    QSpinBox  *m_utilisation = nullptr;
    QDateEdit *m_dInstall = nullptr;
    QDateEdit *m_dDerniere = nullptr;
    QDateEdit *m_dProchaine = nullptr;

    // liste : recherche, filtres, tri
    QLineEdit *m_recherche = nullptr;
    QComboBox *m_filtreType = nullptr;
    QComboBox *m_filtreEtat = nullptr;
    QComboBox *m_triCombo = nullptr;
    QPushButton *m_triOrdreBtn = nullptr;
    int m_triCol = cID;
    Qt::SortOrder m_triOrdre = Qt::AscendingOrder;
    QTableWidget *m_table = nullptr;

    // maintenance IA + Smart Alert
    QTabWidget *m_onglets = nullptr;
    QComboBox *m_iaEquip = nullptr;
    QProgressBar *m_iaScore = nullptr;
    QLabel *m_iaNiveau = nullptr;
    QLabel *m_iaDate = nullptr;
    QLabel *m_iaPanne = nullptr;
    QLabel *m_iaMotifs = nullptr;
    QPushButton *m_iaAppliquer = nullptr;
    QListWidget *m_iaClassement = nullptr;
    QListWidget *m_alertes = nullptr;
    int m_iaJourProposee = 0;

    // données en mémoire
    QList<Equipement> m_equipements;
    QList<Evenement> m_evenements;
    bool m_historiqueOk = false;
    QString m_historiqueErreur;
};

#endif // GESTION_EQUIPEMENT_H
