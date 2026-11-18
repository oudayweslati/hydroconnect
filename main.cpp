#include "connexion.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");          // rendu identique sur tous les PC
    app.setFont(QFont("Segoe UI", 9));

    // Connexion Oracle (ODBC). En cas d'échec l'application s'ouvre quand même,
    // avec le message d'erreur pour faciliter le diagnostic.
    QString erreur;
    const bool connecte = Connexion::ouvrir(erreur);

    MainWindow window;
    window.show();

    if (!connecte) {
        QMessageBox::warning(&window, "Connexion Oracle impossible",
                             "Impossible de se connecter à la base Oracle.\n\n" + erreur +
                             "\n\nVérifiez le DSN, l'utilisateur et le mot de passe dans connexion.cpp.");
    }
    return app.exec();
}
