#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QWidget;

// Fenêtre principale : menu latéral (logo + navigation) et zone de pages.
// Chaque module de l'équipe (Employés, Produits, Clients, Usines...) est une
// page QWidget ajoutée dans le QStackedWidget.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QWidget *buildSidebar();
    QWidget *makePlaceholder(const QString &nom);

    QStackedWidget *m_stack = nullptr;
};

#endif // MAINWINDOW_H
