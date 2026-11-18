#include "xlsx_writer.h"

#include <cstdint>
#include <cstdio>

namespace Xlsx {
namespace {

// ---------------------------------------------------------------- CRC32 + ZIP
uint32_t crc32(const std::string &data)
{
    static uint32_t table[256];
    static bool ready = false;
    if (!ready) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        ready = true;
    }
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned char ch : data)
        crc = table[(crc ^ ch) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

void put16(std::string &s, uint32_t v)
{
    s.push_back(char(v & 0xFF));
    s.push_back(char((v >> 8) & 0xFF));
}

void put32(std::string &s, uint32_t v)
{
    put16(s, v & 0xFFFF);
    put16(s, (v >> 16) & 0xFFFF);
}

struct Entry
{
    std::string name;
    std::string data;
    uint32_t crc = 0;
    uint32_t offset = 0;
};

std::string zipStored(std::vector<Entry> entries)
{
    // date/heure DOS fixes : 01/01/2026 00:00:00
    const uint32_t dosTime = 0;
    const uint32_t dosDate = ((2026 - 1980) << 9) | (1 << 5) | 1;

    std::string out;
    for (Entry &e : entries) {
        e.crc = crc32(e.data);
        e.offset = uint32_t(out.size());
        put32(out, 0x04034B50u);            // signature en-tête local
        put16(out, 20);                     // version nécessaire
        put16(out, 0);                      // drapeaux
        put16(out, 0);                      // méthode : 0 = stocké
        put16(out, dosTime);
        put16(out, dosDate);
        put32(out, e.crc);
        put32(out, uint32_t(e.data.size())); // taille compressée
        put32(out, uint32_t(e.data.size())); // taille d'origine
        put16(out, uint32_t(e.name.size()));
        put16(out, 0);                      // champ extra
        out += e.name;
        out += e.data;
    }

    const uint32_t cdStart = uint32_t(out.size());
    for (const Entry &e : entries) {
        put32(out, 0x02014B50u);            // signature répertoire central
        put16(out, 20);                     // version créée par
        put16(out, 20);                     // version nécessaire
        put16(out, 0);
        put16(out, 0);
        put16(out, dosTime);
        put16(out, dosDate);
        put32(out, e.crc);
        put32(out, uint32_t(e.data.size()));
        put32(out, uint32_t(e.data.size()));
        put16(out, uint32_t(e.name.size()));
        put16(out, 0);                      // extra
        put16(out, 0);                      // commentaire
        put16(out, 0);                      // disque
        put16(out, 0);                      // attributs internes
        put32(out, 0);                      // attributs externes
        put32(out, e.offset);
        out += e.name;
    }
    const uint32_t cdSize = uint32_t(out.size()) - cdStart;

    put32(out, 0x06054B50u);                // fin du répertoire central
    put16(out, 0);
    put16(out, 0);
    put16(out, uint32_t(entries.size()));
    put16(out, uint32_t(entries.size()));
    put32(out, cdSize);
    put32(out, cdStart);
    put16(out, 0);
    return out;
}

// ------------------------------------------------------------------- XML
// Échappe le texte et retire les caractères interdits en XML 1.0.
std::string xmlEscape(const std::string &s)
{
    std::string r;
    r.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
        case '&': r += "&amp;"; break;
        case '<': r += "&lt;"; break;
        case '>': r += "&gt;"; break;
        case '"': r += "&quot;"; break;
        case '\'': r += "&apos;"; break;
        default:
            if (c < 0x20 && c != '\t' && c != '\n' && c != '\r')
                break;                       // caractère de contrôle : ignoré
            r.push_back(char(c));
        }
    }
    return r;
}

// Nom d'onglet valide : 31 caractères max, sans  [ ] : * ? / \ .
std::string sheetName(const std::string &name, size_t index)
{
    std::string r;
    for (unsigned char c : name) {
        if (c == '[' || c == ']' || c == ':' || c == '*' || c == '?' || c == '/' || c == '\\')
            c = ' ';
        r.push_back(char(c));
    }
    // limite en octets prudente : coupe sans casser une séquence UTF-8
    if (r.size() > 31) {
        size_t cut = 31;
        while (cut > 0 && (static_cast<unsigned char>(r[cut]) & 0xC0) == 0x80)
            --cut;
        r.resize(cut);
    }
    if (r.empty())
        r = "Feuille" + std::to_string(index + 1);
    return r;
}

std::string columnName(size_t index)            // 0 -> A, 25 -> Z, 26 -> AA
{
    std::string name;
    size_t n = index + 1;
    while (n > 0) {
        const size_t rem = (n - 1) % 26;
        name.insert(name.begin(), char('A' + rem));
        n = (n - 1) / 26;
    }
    return name;
}

std::string numberText(double v)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.10g", v);
    return buf;
}

const char *kXmlHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

std::string sheetXml(const Sheet &sheet)
{
    std::string x = kXmlHead;
    x += "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">";

    if (!sheet.widths.empty()) {
        x += "<cols>";
        for (size_t i = 0; i < sheet.widths.size(); ++i) {
            x += "<col min=\"" + std::to_string(i + 1) + "\" max=\"" + std::to_string(i + 1)
                 + "\" width=\"" + numberText(sheet.widths[i]) + "\" customWidth=\"1\"/>";
        }
        x += "</cols>";
    }

    x += "<sheetData>";
    for (size_t r = 0; r < sheet.rows.size(); ++r) {
        x += "<row r=\"" + std::to_string(r + 1) + "\">";
        for (size_t c = 0; c < sheet.rows[r].size(); ++c) {
            const Cell &cell = sheet.rows[r][c];
            const std::string ref = columnName(c) + std::to_string(r + 1);
            const int style = cell.header ? 1 : (cell.title ? 2 : 0);
            x += "<c r=\"" + ref + "\"";
            if (style)
                x += " s=\"" + std::to_string(style) + "\"";
            if (cell.isNumber) {
                x += "><v>" + numberText(cell.number) + "</v></c>";
            } else {
                x += " t=\"inlineStr\"><is><t xml:space=\"preserve\">" + xmlEscape(cell.text)
                     + "</t></is></c>";
            }
        }
        x += "</row>";
    }
    x += "</sheetData></worksheet>";
    return x;
}

std::string stylesXml()
{
    std::string x = kXmlHead;
    x += "<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
         "<fonts count=\"3\">"
         "<font><sz val=\"11\"/><name val=\"Calibri\"/></font>"
         "<font><b/><sz val=\"11\"/><color rgb=\"FFFFFFFF\"/><name val=\"Calibri\"/></font>"
         "<font><b/><sz val=\"14\"/><color rgb=\"FF0B2A55\"/><name val=\"Calibri\"/></font>"
         "</fonts>"
         "<fills count=\"3\">"
         "<fill><patternFill patternType=\"none\"/></fill>"
         "<fill><patternFill patternType=\"gray125\"/></fill>"
         "<fill><patternFill patternType=\"solid\"><fgColor rgb=\"FF0B2A55\"/><bgColor indexed=\"64\"/></patternFill></fill>"
         "</fills>"
         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
         "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
         "<cellXfs count=\"3\">"
         "<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"
         "<xf numFmtId=\"0\" fontId=\"1\" fillId=\"2\" borderId=\"0\" xfId=\"0\" applyFont=\"1\" applyFill=\"1\"/>"
         "<xf numFmtId=\"0\" fontId=\"2\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/>"
         "</cellXfs>"
         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>"
         "</styleSheet>";
    return x;
}

} // namespace

std::string build(const std::vector<Sheet> &sheetsIn)
{
    std::vector<Sheet> sheets = sheetsIn;
    if (sheets.empty())
        sheets.push_back(Sheet());           // Excel exige au moins une feuille

    std::vector<Entry> files;
    auto add = [&files](const std::string &name, const std::string &data) {
        Entry e;
        e.name = name;
        e.data = data;
        files.push_back(e);
    };

    // [Content_Types].xml
    std::string types = kXmlHead;
    types += "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
             "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
             "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
             "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
             "<Override PartName=\"/xl/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml\"/>";
    for (size_t i = 0; i < sheets.size(); ++i) {
        types += "<Override PartName=\"/xl/worksheets/sheet" + std::to_string(i + 1)
                 + ".xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>";
    }
    types += "</Types>";
    add("[Content_Types].xml", types);

    // _rels/.rels
    std::string rels = kXmlHead;
    rels += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
            "</Relationships>";
    add("_rels/.rels", rels);

    // xl/workbook.xml
    std::string wb = kXmlHead;
    wb += "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
          "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets>";
    for (size_t i = 0; i < sheets.size(); ++i) {
        wb += "<sheet name=\"" + xmlEscape(sheetName(sheets[i].name, i)) + "\" sheetId=\""
              + std::to_string(i + 1) + "\" r:id=\"rId" + std::to_string(i + 1) + "\"/>";
    }
    wb += "</sheets></workbook>";
    add("xl/workbook.xml", wb);

    // xl/_rels/workbook.xml.rels
    std::string wbRels = kXmlHead;
    wbRels += "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">";
    for (size_t i = 0; i < sheets.size(); ++i) {
        wbRels += "<Relationship Id=\"rId" + std::to_string(i + 1)
                  + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet"
                  + std::to_string(i + 1) + ".xml\"/>";
    }
    wbRels += "<Relationship Id=\"rId" + std::to_string(sheets.size() + 1)
              + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>";
    wbRels += "</Relationships>";
    add("xl/_rels/workbook.xml.rels", wbRels);

    add("xl/styles.xml", stylesXml());
    for (size_t i = 0; i < sheets.size(); ++i)
        add("xl/worksheets/sheet" + std::to_string(i + 1) + ".xml", sheetXml(sheets[i]));

    return zipStored(files);
}

} // namespace Xlsx
