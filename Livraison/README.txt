HydroConnect - module Livraison seul (C++ / Qt, POO)
====================================================
Architecture Modele-Vue :
- livraison.h/.cpp            Modele : classe Livraison (attributs prives, getters/setters, valider(),
                              CRUD ajouter/modifier/supprimer/afficher, statistiques, erreur()).
                              Requetes preparees uniquement ; ajout avec ID deja existant refuse.
- gestion_livraison.h/.cpp    Vue : formulaire, QTableView, details, statistiques (aucun SQL).
- livraison_donutwidget.*     Graphique en anneau.
- connection.h/.cpp           Singleton de Meyers (QODBC). Variables d'environnement
                              HYDROCONNECT_DB_DSN / _USER / _PASSWORD (defaut : tasnim / tasnim / tasnim2006).
- schema.h/.cpp               Cree CLIENT et LIVRAISON + sequences si absentes (CLIENT sert de cle etrangere).
- fenetre_principale.*        Fenetre principale : menu lateral (Accueil, Employes, Produits, Equipements,
                              Usines, Clients, Livraisons, Statistiques, Exportation, Parametres).
                              Seule la page Livraisons est realisee ; les autres pages sont vides.
- resources.qrc / images      Logo HydroConnect du menu.
- main.cpp                    Connexion puis affichage de la fenetre principale (page Livraisons par defaut).
- base_livraison_oracle.sql   Script SQL equivalent.

Ouvrir CMakeLists.txt ou HydroConnectLivraison.pro dans Qt Creator (Qt 6, MinGW), puis Nettoyer / Reconfigurer / Compiler.
Les autres modules (Employe, Usine, Equipement, Produit, Client, connexion) ont ete retires.
