QT += core gui widgets sql
CONFIG += c++17
TARGET = HydroConnectLivraison
TEMPLATE = app

SOURCES += \
    main.cpp \
    fenetre_principale.cpp \
    connection.cpp \
    schema.cpp \
    livraison.cpp \
    gestion_livraison.cpp \
    livraison_donutwidget.cpp

HEADERS += \
    fenetre_principale.h \
    connection.h \
    schema.h \
    livraison.h \
    gestion_livraison.h \
    livraison_donutwidget.h \
    responsive.h

RESOURCES += resources.qrc
