-- =====================================================================
--  Historique des pannes et maintenances (maintenance prédictive)
--  À exécuter UNE FOIS dans SQL Developer, connexion "tasnim".
--  Exécuter avec F5 (Exécuter le script).
-- =====================================================================

CREATE TABLE HISTORIQUE_EQUIPEMENT (
    ID_HISTORIQUE   NUMBER         PRIMARY KEY,
    ID_EQUIPEMENT   NUMBER         NOT NULL,
    TYPE_EVENEMENT  VARCHAR2(20)   NOT NULL,
    DATE_EVENEMENT  DATE           NOT NULL,
    DESCRIPTION     VARCHAR2(200),
    CONSTRAINT CHK_TYPE_EVENEMENT CHECK (TYPE_EVENEMENT IN ('Panne', 'Maintenance'))
);

CREATE INDEX IDX_HISTORIQUE_EQUIP ON HISTORIQUE_EQUIPEMENT (ID_EQUIPEMENT);

-- Vérification :
SELECT column_name, data_type, nullable
FROM   user_tab_columns
WHERE  table_name = 'HISTORIQUE_EQUIPEMENT'
ORDER  BY column_id;

-- ---------------------------------------------------------------------
--  (Facultatif) Jeu de données de démonstration pour tester l'IA.
--  Remplacez 1 par l'ID_EQUIPEMENT d'un équipement existant, puis
--  décommentez (supprimez les "-- ") et exécutez.
-- ---------------------------------------------------------------------
-- INSERT INTO HISTORIQUE_EQUIPEMENT VALUES (1, 1, 'Panne',       SYSDATE - 200, 'Roulement usé');
-- INSERT INTO HISTORIQUE_EQUIPEMENT VALUES (2, 1, 'Panne',       SYSDATE - 120, 'Surchauffe moteur');
-- INSERT INTO HISTORIQUE_EQUIPEMENT VALUES (3, 1, 'Panne',       SYSDATE - 50,  'Fuite joint');
-- INSERT INTO HISTORIQUE_EQUIPEMENT VALUES (4, 1, 'Maintenance', SYSDATE - 300, 'Révision annuelle');
-- INSERT INTO HISTORIQUE_EQUIPEMENT VALUES (5, 1, 'Maintenance', SYSDATE - 130, 'Remplacement roulement');
-- COMMIT;
