#ifndef XLSX_WRITER_H
#define XLSX_WRITER_H

#include <string>
#include <vector>

// Générateur minimal de fichiers Excel (.xlsx) sans bibliothèque externe.
// Un .xlsx est une archive ZIP de fichiers XML : on écrit une archive "stockée"
// (sans compression), que Excel / LibreOffice ouvrent normalement.
// Ce module n'utilise que la bibliothèque standard C++ (pas de Qt).
namespace Xlsx {

struct Cell
{
    std::string text;        // texte UTF-8 (si isNumber == false)
    double number = 0;       // valeur (si isNumber == true)
    bool isNumber = false;
    bool header = false;     // style "en-tête" : gras, blanc sur fond bleu marine
    bool title = false;      // style "titre" : gras, grande taille

    static Cell str(const std::string &s) { Cell c; c.text = s; return c; }
    static Cell num(double v) { Cell c; c.number = v; c.isNumber = true; return c; }
    static Cell head(const std::string &s) { Cell c; c.text = s; c.header = true; return c; }
    static Cell titre(const std::string &s) { Cell c; c.text = s; c.title = true; return c; }
};

struct Sheet
{
    std::string name;                         // nom de l'onglet (31 caractères max)
    std::vector<double> widths;               // largeur des colonnes (optionnel)
    std::vector<std::vector<Cell>> rows;
};

// Retourne le contenu binaire complet du fichier .xlsx.
std::string build(const std::vector<Sheet> &sheets);

} // namespace Xlsx

#endif // XLSX_WRITER_H
