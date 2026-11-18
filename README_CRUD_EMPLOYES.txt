HYDROCONNECT - GESTION DES EMPLOYES ET CHATBOT LOCAL

Mise en place :
1. Dans connection.cpp : verifier le nom de la source ODBC, l'utilisateur et le mot de passe Oracle.
2. SQL Developer : executer sql/adaptation_table_employe.sql pour creer la sequence SEQ_EMPLOYE et le trigger de generation des identifiants si necessaires.
3. Qt Creator : ouvrir CMakeLists.txt, choisir un kit Qt compatible, compiler et lancer.

Fonctionnalites conservees :
- Connexion a l'application HydroConnect.
- Gestion des employes : Ajouter / Modifier / Supprimer / Afficher depuis Oracle.
- Recherche par ID, nom, prenom, email, poste et departement.
- Tri par nom, age, poste et departement.
- Export Excel et statistiques par age, poste et departement.
- Rubriques Produits et Equipements.

Nouvelle fonctionnalite : Chatbot HydroConnect (HydroBot)
- Accessible depuis le menu lateral Chatbot.
- Interface de conversation avec zone de saisie et bouton Envoyer.
- Reponses locales sur l'utilisation de l'application, les employes, la recherche, le tri, l'export, les statistiques, Oracle, les produits et les equipements.
- Fonctionne sans Internet, sans cle API et sans service externe. Il s'agit d'un assistant a regles, pas d'un modele d'IA generative.

Fonctionnalites retirees :
- Carte d'employe avec QR Code.
- Generation et lecture / scan de QR Code.
- Bandeau de scan en bas de la page Employes.
- Ancien bouton de detection des absences.
