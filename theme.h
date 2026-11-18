#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QPalette>
#include <QString>

// ============================================================
// THEME HYDROCONNECT (repris de la Gestion des équipements)
//   Navy  #0B2A55   Bleu #2F6FED   Vert #1FA971   Fond #F6F9FF
// ============================================================

namespace Theme {

inline QPalette palette()
{
    QPalette p;
    const QColor navy("#0B2A55");
    p.setColor(QPalette::Window, QColor("#F6F9FF"));
    p.setColor(QPalette::WindowText, navy);
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::AlternateBase, QColor("#F5F9FF"));
    p.setColor(QPalette::Text, navy);
    p.setColor(QPalette::Button, Qt::white);
    p.setColor(QPalette::ButtonText, navy);
    p.setColor(QPalette::Highlight, QColor("#DCE9FF"));
    p.setColor(QPalette::HighlightedText, navy);
    p.setColor(QPalette::ToolTipBase, Qt::white);
    p.setColor(QPalette::ToolTipText, navy);
    return p;
}

// Style de la page "Gestion des produits" (mêmes couleurs/formes que les équipements)
inline QString styleProduit()
{
    return QStringLiteral(R"CSS(
        QWidget#GestionProduit { background:#F6F9FF; }
        QWidget { color:#0B2A55; font-family:'Segoe UI'; font-size:13px; }
        QLabel { background:transparent; }

        QLabel#pageTitle    { color:#0B2A55; font-size:26px; font-weight:700; }
        QLabel#pageSubtitle { color:#6C7C98; font-size:13px; }
        QLabel#panelTitle   { color:#0B2A55; font-size:15px; font-weight:700; }
        QLabel#smallLabel   { color:#6C7C98; font-size:12px; }
        QLabel#valueLabel   { color:#0B2A55; font-size:13px; font-weight:700; }
        QLabel#percent      { color:#2F6FED; font-size:12px; font-weight:700; }

        QFrame#panel     { background:white; border:1px solid #E1EAF7; border-radius:16px; }
        QFrame#innerCard { background:#F4F8FF; border:1px solid #DCE6F5; border-radius:14px; }

        QLineEdit, QComboBox, QDateEdit {
            background:white; border:1px solid #CFDDF2; border-radius:10px;
            padding:7px 10px; color:#0B2A55; min-height:22px;
            selection-background-color:#2F6FED; selection-color:white;
        }
        QLineEdit:hover, QComboBox:hover, QDateEdit:hover { border:1px solid #9DB8E6; }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border:1px solid #2F6FED; }
        QComboBox QAbstractItemView {
            background:white; color:#0B2A55; border:1px solid #CFDDF2;
            selection-background-color:#DCE9FF; selection-color:#0B2A55; outline:0;
        }

        QPushButton { border:none; border-radius:10px; padding:9px 16px; font-weight:700; }
        QPushButton#primary         { background:#2F6FED; color:white; }
        QPushButton#primary:hover   { background:#2461D6; }
        QPushButton#warning         { background:#EAF2FF; color:#2F6FED; }
        QPushButton#warning:hover   { background:#DCE9FF; }
        QPushButton#danger          { background:white; color:#0B2A55; border:1px solid #CFDDF2; }
        QPushButton#danger:hover    { background:#F4F8FF; }

        QTableWidget { background:white; alternate-background-color:#F5F9FF; border:none;
            gridline-color:#EEF3FB; selection-background-color:#DCE9FF; selection-color:#0B2A55; }
        QTableWidget::item { padding:4px 8px; border-bottom:1px solid #EEF3FB; }
        QHeaderView { background:#EAF2FF; }
        QHeaderView::section { background:#EAF2FF; color:#0B2A55; border:none;
            padding:10px 8px; font-weight:700; }
        QTableCornerButton::section { background:#EAF2FF; border:none; }

        QScrollBar:vertical { background:transparent; width:10px; margin:2px; }
        QScrollBar::handle:vertical { background:#C9D8F0; border-radius:4px; min-height:30px; }
        QScrollBar:horizontal { background:transparent; height:10px; margin:2px; }
        QScrollBar::handle:horizontal { background:#C9D8F0; border-radius:4px; min-width:30px; }
        QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }
        QScrollBar::add-page, QScrollBar::sub-page { background:transparent; }

        QProgressBar { border:none; background:#E6EEFA; border-radius:7px; text-align:center;
            color:#0B2A55; min-height:14px; max-height:14px; }
        QProgressBar::chunk { background:#2F6FED; border-radius:7px; }
    )CSS");
}

// Style de la fenêtre principale + sidebar de navigation
inline QString styleApp()
{
    return QStringLiteral(R"CSS(
        QMainWindow { background:#F6F9FF; }

        QWidget#sidebar { background:#0B2A55; }
        QLabel#logoText { color:white; font-size:18px; font-weight:700; background:transparent; }
        QLabel#logoSub  { color:#1FA971; font-size:11px; font-weight:700; background:transparent; }

        QPushButton#nav {
            background:transparent; color:#C9D8F0; text-align:left;
            padding:12px 14px; border:none; border-radius:10px;
            font-size:13px; font-weight:600;
        }
        QPushButton#nav:hover { background:#123A73; color:white; }
        QPushButton#navActive {
            background:#2F6FED; color:white; text-align:left;
            padding:12px 14px; border:none; border-radius:10px;
            font-size:13px; font-weight:700;
        }
        QLabel#etatSysteme {
            background:#E4F6EE; color:#14865A; border-radius:10px;
            padding:9px; font-weight:700; font-size:11px;
        }
    )CSS");
}

} // namespace Theme

#endif // THEME_H
