#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Recuperation de l'instance unique du singleton
    Connection &c = Connection::createInstance();

    // Tentative d'etablissement de la connexion
    bool test = c.createConnection();

    MainWindow window;

    if (test) {
        window.show();
        QMessageBox::information(nullptr, QObject::tr("Base de donnees"),
                                 QObject::tr("Connexion reussie.\nCliquez sur Annuler pour quitter."),
                                 QMessageBox::Cancel);
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Base de donnees"),
                              QObject::tr("Echec de la connexion.\nCliquez sur Annuler pour quitter."),
                              QMessageBox::Cancel);
        return 1;   // pas de connexion : on quitte
    }

    return app.exec();
}
