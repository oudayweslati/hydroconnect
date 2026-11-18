#include "hydroconnect.h"
#include "connection.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!Connection::createConnection()) {
        QMessageBox::critical(nullptr, "HydroConnect", "Connexion a la base de donnees impossible.");
        return 1;
    }

    HydroConnect w;
    w.show();
    return a.exec();
}
