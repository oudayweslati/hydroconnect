-- HydroConnect : adaptation minimale de la table EMPLOYE
-- A executer avec l'utilisateur proprietaire de la table EMPLOYE dans SQL Developer (F5).
-- Ce script ne supprime aucune donnee et ne cree aucune table QR / emploi du temps.

-- ID_USINE devient facultatif si votre modele autorise un employe sans usine.
BEGIN
    EXECUTE IMMEDIATE 'ALTER TABLE EMPLOYE MODIFY ID_USINE NULL';
EXCEPTION
    WHEN OTHERS THEN
        IF SQLCODE != -1451 THEN RAISE; END IF;
END;
/

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*) INTO n FROM user_sequences WHERE sequence_name = 'SEQ_EMPLOYE';
    IF n = 0 THEN
        SELECT NVL(MAX(ID_EMPLOYE), 0) + 1 INTO n FROM EMPLOYE;
        EXECUTE IMMEDIATE 'CREATE SEQUENCE SEQ_EMPLOYE START WITH ' || n || ' INCREMENT BY 1 NOCACHE';
    END IF;
END;
/

CREATE OR REPLACE TRIGGER TRG_EMPLOYE
BEFORE INSERT ON EMPLOYE FOR EACH ROW
BEGIN
    IF :NEW.ID_EMPLOYE IS NULL THEN
        SELECT SEQ_EMPLOYE.NEXTVAL INTO :NEW.ID_EMPLOYE FROM DUAL;
    END IF;
END;
/

COMMIT;

-- Verification
SELECT column_name FROM user_tab_columns WHERE table_name = 'EMPLOYE' ORDER BY column_id;
SELECT sequence_name FROM user_sequences WHERE sequence_name = 'SEQ_EMPLOYE';
