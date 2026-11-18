#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

class GestionEmploye;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void togglePasswordVisibility();
    void handleLogin();
    void showForgotMessage();
    void startFingerprint();

private:
    QWidget* buildLoginCard();
    void openApp(const QString &user);

    QLineEdit   *m_userEdit     = nullptr;
    QLineEdit   *m_passEdit     = nullptr;
    QPushButton *m_eyeButton    = nullptr;
    QPushButton *m_loginButton  = nullptr;
    QCheckBox   *m_rememberBox  = nullptr;
    QLabel      *m_messageLabel = nullptr;
    GestionEmploye *m_employeeWindow = nullptr;
    bool m_fpBusy = false;
};

#endif