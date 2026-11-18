-- =====================================================================
--  Script à exécuter dans SQL Developer (connexion TASNIM_FINAL)
--  Table : EQUIPEMENT
-- =====================================================================

-- 1) ID_USINE était obligatoire alors que le module Equipements ne le
--    renseigne pas : on le rend facultatif.
ALTER TABLE EQUIPEMENT MODIFY (ID_USINE NULL);

-- 2) Supprimer les colonnes en double ajoutées par erreur (elles sont vides).
--    Vérification préalable (doit afficher 0 partout) :
SELECT COUNT(DESIGNATION) d, COUNT(TYPE_EQUIPEMENT) t, COUNT(UTILISATION) u,
       COUNT(DATE_DERNIERE_MAINT) dm, COUNT(DATE_PROCHAINE_MAINT) pm
FROM   EQUIPEMENT;

ALTER TABLE EQUIPEMENT DROP (DESIGNATION, TYPE_EQUIPEMENT, UTILISATION,
                             DATE_DERNIERE_MAINT, DATE_PROCHAINE_MAINT);

-- 3) Vérification finale : 12 colonnes d'origine attendues
SELECT column_name, data_type, nullable
FROM   user_tab_columns
WHERE  table_name = 'EQUIPEMENT'
ORDER  BY column_id;
