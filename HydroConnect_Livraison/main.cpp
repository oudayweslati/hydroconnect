#include "fenetre_principale.h"
#include "connection.h"
#include "schema.h"
#include "responsive.h"

#include <QApplication>
#include <QMessageBox>
#include <QFont>

int main(int argc, char *argv[])
{
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);

    // Police de base adaptee a la resolution de l'ecran.
    QFont baseFont = app.font();
    if (baseFont.pointSizeF() > 0)
        baseFont.setPointSizeF(qMax(8.0, baseFont.pointSizeF() * Responsive::factor()));
    app.setFont(baseFont);

    // Instance unique de la connexion (Singleton de Meyers).
    Connection &c = Connection::createInstance();
    if (c.createConnection()) {
        SchemaBD::assurer();        // cree CLIENT / LIVRAISON / sequences si absentes
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion.\n%1\n\nL'application s'ouvre quand même ; "
                        "la liste restera vide tant que la base n'est pas disponible.")
                .arg(c.lastErrorText()));
    }

    FenetrePrincipale window;
    window.resize(1700, 900);
    Responsive::apply(&window);
    window.show();
    return app.exec();
}
