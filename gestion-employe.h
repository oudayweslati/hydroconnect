#ifndef GESTION_EMPLOYE_H
#define GESTION_EMPLOYE_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QDate>
#include <QStackedWidget>
class QTableWidget;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QDateEdit;
class QLabel;
class QPushButton;
class QTextEdit;
class QProgressBar;
class QPaintEvent;
class QFrame;
class QWidget;

class GestionEmploye; // Déclaration anticipée pour l'accès contrôlé aux données

class Employe
{
private:
    // Attributs privés : protégés contre l'accès direct depuis l'extérieur
    int id = 0;
    QString nom;
    QString prenom;
    int age = 0;
    QString email;
    QString telephone;
    QString poste;
    QString departement;
    QDate dateEmbauche;
    int idUsine = 0;

    // La classe de gestion peut manipuler les données pour conserver
    // le fonctionnement actuel de l'application HydroConnect.
    friend class GestionEmploye;

public:
    Employe() = default;

    // Getters : lecture des attributs privés
    int getId() const { return id; }
    QString getNom() const { return nom; }
    QString getPrenom() const { return prenom; }
    int getAge() const { return age; }
    QString getEmail() const { return email; }
    QString getTelephone() const { return telephone; }
    QString getPoste() const { return poste; }
    QString getDepartement() const { return departement; }
    QDate getDateEmbauche() const { return dateEmbauche; }
    int getIdUsine() const { return idUsine; }

    // Setters : modification contrôlée des attributs privés
    void setId(int valeur) { id = valeur; }
    void setNom(const QString &valeur) { nom = valeur; }
    void setPrenom(const QString &valeur) { prenom = valeur; }
    void setAge(int valeur) { age = valeur; }
    void setEmail(const QString &valeur) { email = valeur; }
    void setTelephone(const QString &valeur) { telephone = valeur; }
    void setPoste(const QString &valeur) { poste = valeur; }
    void setDepartement(const QString &valeur) { departement = valeur; }
    void setDateEmbauche(const QDate &valeur) { dateEmbauche = valeur; }
    void setIdUsine(int valeur) { idUsine = valeur; }
};

class BarChart : public QWidget
{
    Q_OBJECT
public:
    explicit BarChart(QWidget *parent = nullptr);
    void setData(const QVector<double> &values, const QVector<QString> &labels, const QString &title);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<double> values;
    QVector<QString> labels;
    QString title;
};

class EmployePieChart : public QWidget
{
    Q_OBJECT
public:
    explicit EmployePieChart(QWidget *parent = nullptr);
    void setData(const QVector<double> &values, const QVector<QString> &labels);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<double> values;
    QVector<QString> labels;
};

class EmployeBottleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EmployeBottleWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};

class GestionEmploye : public QMainWindow
{
    Q_OBJECT
public:
    explicit GestionEmploye(QWidget *parent = nullptr);

private:
    QVector<Employe> employes;
    QStackedWidget *m_contentStack = nullptr;
    QWidget *m_employeeContent = nullptr;
    QWidget *m_chatbotContent = nullptr;
    QWidget *m_statsContent = nullptr;
    QTextEdit *m_chatHistory = nullptr;
    QLineEdit *m_chatInput = nullptr;

    QLineEdit *searchEdit = nullptr;
    QComboBox *triCombo = nullptr;
    QComboBox *departementFilter = nullptr;
    QTableWidget *table = nullptr;

    QLineEdit *idEdit = nullptr;
    QLineEdit *nomEdit = nullptr;
    QLineEdit *prenomEdit = nullptr;
    QSpinBox *ageSpin = nullptr;
    QLineEdit *emailEdit = nullptr;
    QLineEdit *telephoneEdit = nullptr;
    QComboBox *posteCombo = nullptr;
    QComboBox *departementCombo = nullptr;
    QDateEdit *dateEdit = nullptr;
    int defaultUsineId = 0;    // ID_USINE utilisé pour les nouveaux employés (colonne obligatoire en base)

    QLabel *totalLabel = nullptr;
    QLabel *ageMoyenLabel = nullptr;

    EmployePieChart *pieChart = nullptr;
    QComboBox *statsMode = nullptr;

    void setupUI();
    void setupStyle();
    void loadUsines();         // lit l'usine par défaut dans Oracle (plus petit ID_USINE de la table USINE)
    bool loadFromDatabase();   // charge les employes depuis Oracle (avec le tri choisi)
    void reload();             // recharge depuis Oracle puis rafraichit le tableau
    QWidget *createSidebar();
    QWidget *createTopCards();
    QWidget *createTablePanel();
    QWidget *createStatsPanel();
    QWidget *createBottomPanels();
    QWidget *createChatbotPage();
    QString chatbotReply(const QString &message) const;
    QWidget *createFormPanel();
    QWidget *createCard(const QString &title, const QString &value, const QString &icon, const QString &bg);
    QPushButton *button(const QString &text, const QString &name);

    void refreshTable();
    void updateStatistics();
    void updateCharts();
    int selectedId() const;
    int indexById(int id) const;
    int nextId() const;
    bool readForm(Employe &e, bool checkId = true);
    void fillForm(const Employe &e);
    void clearForm();

private slots:
    void addEmploye();
    void modifyEmploye();
    void deleteEmploye();
    void selectEmploye();
    void searchEmployes();
    void sortEmployes();
    void exportEmployes();
    void showStatistics();
    void sendChatMessage();
};

#endif
