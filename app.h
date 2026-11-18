#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QVector>
#include <QString>
#include <QColor>
#include <QDate>
#include <functional>

class QTableWidget;
class QListWidget;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QDateEdit;
class QLabel;
class QPushButton;
class QProgressBar;
class QTimer;
class QStackedWidget;
class QPaintEvent;

// ============================================================
// DONNEES
// ============================================================

struct Produit
{
    int id;
    QString nom;
    QString type;
    double prix;
    int stock;
    QDate dateProd;
    QDate dateExp;
};

struct Equipement
{
    int id;
    QString designation;
    QString type;
    QString marque;
    QString modele;
    QDate dateInstallation;
    QString localisation;
    QString etat;
    int niveauUtilisation;
    QDate derniereMaintenance;
    QDate prochaineMaintenance;
    QVector<QDate> historiquePannes;
};

struct Client
{
    int id;
    QString nom;
    QString email;
    QString telephone;
    QString categorie;
    double chiffreAffaires;
};

struct Employe
{
    int id;
    QString nom;
    QString prenom;
    int age;
    QString email;
    QString telephone;
    QString poste;
    QString departement;
    QDate dateEmbauche;
    QString statut;
    QVector<QDate> absences;
};

struct RiskResult
{
    int score = 0;
    QString niveau;
    QDate dateProposee;
    int count12m = 0;
};

// level : 0 = rouge, 1 = orange, 2 = violet
struct AlertItem
{
    int level;
    QString text;
};

// ============================================================
// WIDGETS PARTAGES
// ============================================================

class BarChart : public QWidget
{
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);
    void setTitle(const QString &title);
    void setColors(const QVector<QColor> &colors);
    void setData(const QVector<double> &values, const QVector<QString> &labels);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_title;
    QVector<QColor> m_colors;
    QVector<double> m_values;
    QVector<QString> m_labels;
};

class PieChart : public QWidget
{
    Q_OBJECT
public:
    explicit PieChart(QWidget *parent = nullptr);
    void setTitle(const QString &title);
    void setColors(const QVector<QColor> &colors);
    void setData(const QVector<double> &values, const QVector<QString> &labels);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_title;
    QVector<QColor> m_colors;
    QVector<double> m_values;
    QVector<QString> m_labels;
};

class BottleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BottleWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class StockCircle : public QWidget
{
    Q_OBJECT
public:
    explicit StockCircle(QWidget *parent = nullptr);
    void setValues(int faible, int moyen, int bon);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_faible = 0;
    int m_moyen = 0;
    int m_bon = 0;
};

// Barre de messages avec boutons "+" (confirmer) et "-" (annuler)
class StatusBox : public QFrame
{
    Q_OBJECT
public:
    enum Kind { Info, Success, Error, Confirm };

    explicit StatusBox(const QString &hint, QWidget *parent = nullptr);

    void say(const QString &text, Kind kind);
    void ask(const QString &text, std::function<void()> action);
    void cancel(bool silent);
    void reset();

private:
    QString m_hint;
    QLabel *m_label = nullptr;
    QPushButton *m_ok = nullptr;
    QPushButton *m_no = nullptr;
    std::function<void()> m_action;
};

// Page de base : chaque module est une page de la fenetre principale
class Page : public QWidget
{
    Q_OBJECT
public:
    explicit Page(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual QString summary() const = 0;
};

// ============================================================
// MODULE PRODUITS
// ============================================================

class ProduitPage : public Page
{
    Q_OBJECT
public:
    explicit ProduitPage(QWidget *parent = nullptr);
    QString summary() const override;

private:
    QVector<Produit> produits;

    QTableWidget *table = nullptr;
    QLineEdit *searchEdit = nullptr;
    QComboBox *filtreCombo = nullptr;

    QLineEdit *idEdit = nullptr;
    QLineEdit *nomEdit = nullptr;
    QComboBox *typeCombo = nullptr;
    QDoubleSpinBox *prixSpin = nullptr;
    QSpinBox *qteSpin = nullptr;
    QDateEdit *prodEdit = nullptr;
    QDateEdit *expEdit = nullptr;

    QLabel *dNom = nullptr;
    QLabel *dType = nullptr;
    QLabel *dPrix = nullptr;
    QLabel *dStock = nullptr;
    QLabel *dProd = nullptr;
    QLabel *dExp = nullptr;
    QLabel *stFaible = nullptr;
    QLabel *stMoyen = nullptr;
    QLabel *stBon = nullptr;
    QLabel *lblDate = nullptr;
    QLabel *lblHeure = nullptr;
    QLabel *pct[3] = {nullptr, nullptr, nullptr};
    QProgressBar *bars[3] = {nullptr, nullptr, nullptr};
    StockCircle *circle = nullptr;
    StatusBox *status = nullptr;

    void load();
    void build();
    void refresh();
    void showDetails(const Produit *p);
    void updatePanels();
    void clearForm();
    void fillForm(const Produit &p);
    bool readForm(Produit &out, bool isNew);
    const Produit *find(int id) const;
    void addItem();
    void modifyItem();
    void removeItem();
    void exportCsv();
};

// ============================================================
// MODULE EQUIPEMENTS
// ============================================================

class EquipementPage : public Page
{
    Q_OBJECT
public:
    explicit EquipementPage(QWidget *parent = nullptr);
    QString summary() const override;

private:
    QVector<Equipement> equipements;

    QLineEdit *searchEdit = nullptr;
    QComboBox *triCombo = nullptr;
    QComboBox *etatFilter = nullptr;
    QTableWidget *table = nullptr;

    QLineEdit *idEdit = nullptr;
    QLineEdit *designationEdit = nullptr;
    QComboBox *typeCombo = nullptr;
    QLineEdit *marqueEdit = nullptr;
    QLineEdit *modeleEdit = nullptr;
    QLineEdit *localisationEdit = nullptr;
    QComboBox *etatCombo = nullptr;
    QSpinBox *utilisationSpin = nullptr;
    QDateEdit *installationEdit = nullptr;
    QDateEdit *derniereMaintEdit = nullptr;
    QDateEdit *prochaineMaintEdit = nullptr;

    StatusBox *status = nullptr;

    QLabel *totalLabel = nullptr;
    QLabel *opLabel = nullptr;
    QLabel *panneLabel = nullptr;
    QLabel *maintLabel = nullptr;
    QLabel *dispoLabel = nullptr;

    QComboBox *predictCombo = nullptr;
    QProgressBar *riskBar = nullptr;
    QLabel *riskLabel = nullptr;
    QLabel *riskLevelLabel = nullptr;
    QLabel *predictedDateLabel = nullptr;
    QLabel *predictedInfoLabel = nullptr;

    QFrame *alertPanel = nullptr;
    QLabel *alertBadge = nullptr;
    QListWidget *alertList = nullptr;
    QTimer *alertTimer = nullptr;
    QTimer *blinkTimer = nullptr;
    bool blinkOn = false;
    int lastAlertCount = 0;

    BarChart *barChart = nullptr;
    PieChart *pieChart = nullptr;

    void load();
    void build();
    QWidget *buildForm();
    void refreshTable();
    void updateStatistics();
    void updatePredictCombo();
    void clearForm();
    void fillForm(const Equipement &e);
    bool readForm(Equipement &out, bool isNew);
    const Equipement *find(int id) const;
    void addItem();
    void modifyItem();
    void removeItem();
    void exportCsv();
    RiskResult analyseRisk(const Equipement &e) const;
    void runPrediction();
    QVector<AlertItem> buildAlerts() const;
    void refreshAlerts(bool allowSound);
};

// ============================================================
// MODULE CLIENTS
// ============================================================

class ClientPage : public Page
{
    Q_OBJECT
public:
    explicit ClientPage(QWidget *parent = nullptr);
    QString summary() const override;

private:
    QVector<Client> clients;

    QLineEdit *searchEdit = nullptr;
    QComboBox *triCombo = nullptr;
    QComboBox *categorieFilter = nullptr;
    QTableWidget *table = nullptr;

    QLineEdit *idEdit = nullptr;
    QLineEdit *nomEdit = nullptr;
    QLineEdit *emailEdit = nullptr;
    QLineEdit *telephoneEdit = nullptr;
    QComboBox *categorieCombo = nullptr;
    QDoubleSpinBox *caSpin = nullptr;

    StatusBox *status = nullptr;

    QLabel *nombreLabel = nullptr;
    QLabel *caTotalLabel = nullptr;
    QLabel *grossistesLabel = nullptr;
    QLabel *detaillantsLabel = nullptr;
    QLabel *entreprisesLabel = nullptr;

    QProgressBar *scoreBar = nullptr;
    QLabel *scoreLabel = nullptr;
    QLabel *remiseLabel = nullptr;
    QLabel *roiLabel = nullptr;
    QComboBox *clientFideliteCombo = nullptr;
    QComboBox *clientRoiCombo = nullptr;
    QDoubleSpinBox *budgetSpin = nullptr;
    QDoubleSpinBox *hausseSpin = nullptr;

    BarChart *barChart = nullptr;
    PieChart *pieChart = nullptr;

    void load();
    void build();
    QWidget *buildForm();
    void refreshTable();
    void updateStatistics();
    void clearForm();
    void fillForm(const Client &c);
    bool readForm(Client &out, bool isNew);
    const Client *find(int id) const;
    void addItem();
    void modifyItem();
    void removeItem();
    void exportCsv();
    void calculateFidelity();
    void calculateROI();
};

// ============================================================
// MODULE EMPLOYES
// ============================================================

class EmployePage : public Page
{
    Q_OBJECT
public:
    explicit EmployePage(QWidget *parent = nullptr);
    QString summary() const override;

private:
    QVector<Employe> employes;

    QLineEdit *searchEdit = nullptr;
    QComboBox *triCombo = nullptr;
    QComboBox *depFilter = nullptr;
    QTableWidget *table = nullptr;

    QLineEdit *idEdit = nullptr;
    QLineEdit *nomEdit = nullptr;
    QLineEdit *prenomEdit = nullptr;
    QSpinBox *ageSpin = nullptr;
    QLineEdit *emailEdit = nullptr;
    QLineEdit *telEdit = nullptr;
    QComboBox *posteCombo = nullptr;
    QComboBox *depCombo = nullptr;
    QDateEdit *embaucheEdit = nullptr;
    QComboBox *statutCombo = nullptr;

    StatusBox *status = nullptr;

    QLabel *totalLabel = nullptr;
    QLabel *presentsLabel = nullptr;
    QLabel *absentsLabel = nullptr;
    QLabel *congeLabel = nullptr;
    QLabel *tauxLabel = nullptr;

    QComboBox *analyseCombo = nullptr;
    QProgressBar *riskBar = nullptr;
    QLabel *riskLabel = nullptr;
    QLabel *riskLevelLabel = nullptr;
    QLabel *entretienLabel = nullptr;
    QLabel *analyseInfo = nullptr;

    QFrame *alertPanel = nullptr;
    QLabel *alertBadge = nullptr;
    QListWidget *alertList = nullptr;

    BarChart *barChart = nullptr;
    PieChart *pieChart = nullptr;

    void load();
    void build();
    QWidget *buildForm();
    void refreshTable();
    void updateStatistics();
    void clearForm();
    void fillForm(const Employe &e);
    bool readForm(Employe &out, bool isNew);
    const Employe *find(int id) const;
    void addItem();
    void modifyItem();
    void removeItem();
    void exportCsv();
    RiskResult analyse(const Employe &e) const;
    void runAnalysis();
    void refreshAlerts();
};

// ============================================================
// FENETRE PRINCIPALE
// ============================================================

class AppWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack = nullptr;
    QVector<QPushButton *> navButtons;
    QVector<QPushButton *> homeCards;
    QVector<QString> homeTitles;
    QVector<Page *> pages;

    QWidget *createSidebar();
    QWidget *createHomePage();
    void refreshHome();
    void goTo(int index);
};

#endif // APP_H