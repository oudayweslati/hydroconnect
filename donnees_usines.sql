-- ============================================================
-- OPTIONNEL : charger les 6 usines de demonstration dans Oracle
-- (connexion TASNIM). Colonnes = celles de votre table USINE.
-- ============================================================
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (101, 'Station Centrale Rades', 'Tunis', 'Traitement Eau Potable', 45000, 3200, 1250, 'Operationnel', 'Station Centrale Rades', 'Operationnel');
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (102, 'Usine Bizerte Nord', 'Bizerte', 'Dessalement Eau de Mer', 28000, 1950, 2100, 'Operationnel', 'Usine Bizerte Nord', 'Operationnel');
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (103, 'Station Sousse Sud', 'Sousse', 'Epuration Eaux Usees', 35000, 2400, 1400, 'En maintenance', 'Station Sousse Sud', 'En maintenance');
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (104, 'Station Kairouan Centre', 'Kairouan', 'Filtration Eau Douce', 18000, 1200, 850, 'Operationnel', 'Station Kairouan Centre', 'Operationnel');
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (105, 'Usine Gabes Maritime', 'Gabes', 'Traitement Industriel', 22000, 1600, 1750, 'Arret technique', 'Usine Gabes Maritime', 'Arret technique');
INSERT INTO USINE (ID_USINE, NOM, LOCALISATION, TYPE_TRAITEMENT, CAPACITE, DEBIT_ACTUEL, CONSOMMATION, ETAT_OPERATIONNEL, NOM_USINE, ETAT)
VALUES (106, 'Station Nabeul Plage', 'Nabeul', 'Epuration Eaux Usees', 15000, 1100, 720, 'Operationnel', 'Station Nabeul Plage', 'Operationnel');
COMMIT;
