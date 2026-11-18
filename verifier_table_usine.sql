-- Colonnes reelles de la table USINE dans votre schema Oracle
SELECT column_name, data_type, nullable
FROM   user_tab_columns
WHERE  table_name = 'USINE'
ORDER  BY column_id;
