#include "gestion_equipement.h"

#include <QAbstractItemView>
#include <QColor>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGlobal>
#include <algorithm>

#include "historique_dialog.h"
#include "maintenance_predictive.h"
#include "statistiques_dialog.h"
#include "xlsx_writer.h"

// ---------------------------------------------------------------------------
//  Constantes et thème
// ---------------------------------------------------------------------------
namespace {

const char *kFmt = "dd/MM/yyyy";

const QColor kNavy("#0B2A55");
const QColor kBlue("#2F6FED");
const QColor kGreen("#1FA971");

const QStringList kTypes  = {"Pompe", "Filtre", "Embouteilleuse", "Capteur", "Compresseur", "Autre"};
const QStringList kEtats  = {"Opérationnel", "En maintenance", "En panne"};

const QString kStyle = R"CSS(
    QWidget#page, QWidget#content { background:#F6F9FF; }
    QWidget { color:#0B2A55; font-family:'Segoe UI'; font-size:12px; }
    QLabel { background:transparent; }
    QScrollArea { border:none; background:#F6F9FF; }

    QFrame#card { background:white; border:1px solid #E3EBF7; border-radius:14px; }

    QLabel#pageTitle { font-size:22px; font-weight:800; color:#0B2A55; }
    QLabel#pageSub   { font-size:12px; color:#6C7C98; }
    QLabel#date      { font-size:11px; color:#6C7C98; }
    QLabel#heure     { font-size:15px; font-weight:700; color:#0B2A55; }

    QLabel#section    { font-size:14px; font-weight:800; color:#0B2A55; }
    QLabel#fieldLabel { font-size:11px; color:#7D8CA8; }
    QLabel#detailVal  { font-size:13px; font-weight:700; color:#0B2A55; }
    QLabel#legend     { font-size:12px; font-weight:700; }
    QLabel#barName    { font-size:12px; color:#0B2A55; }
    QLabel#barPct     { font-size:12px; font-weight:700; color:#2F6FED; }
    QLabel#iaValue    { font-size:12px; font-weight:700; }

    QLineEdit, QComboBox, QDateEdit, QSpinBox {
        background:white; border:1px solid #D5E1F5; border-radius:9px;
        padding:3px 10px; min-height:22px; color:#0B2A55;
        selection-background-color:#2F6FED; selection-color:white;
    }
    QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus { border:1px solid #2F6FED; }
    QLineEdit:disabled { background:#F1F5FC; color:#7D8CA8; }
    QComboBox QAbstractItemView {
        background:white; color:#0B2A55; border:1px solid #D5E1F5;
        selection-background-color:#DCE9FF; selection-color:#0B2A55; outline:0;
    }

    QTableWidget { background:white; alternate-background-color:#F5F9FF; border:none;
        selection-background-color:#DCE9FF; selection-color:#0B2A55; gridline-color:transparent; }
    QHeaderView { background:#E8F0FD; border:none; }
    QHeaderView::section { background:#E8F0FD; color:#0B2A55; border:none;
        padding:8px 6px; font-weight:800; font-size:12px; }
    QTableCornerButton::section { background:#E8F0FD; border:none; }

    QScrollBar:vertical { background:transparent; width:9px; margin:2px; }
    QScrollBar::handle:vertical { background:#C9D8F0; border-radius:4px; min-height:30px; }
    QScrollBar:horizontal { background:transparent; height:9px; margin:2px; }
    QScrollBar::handle:horizontal { background:#C9D8F0; border-radius:4px; min-width:30px; }
    QScrollBar::add-line, QScrollBar::sub-line { width:0; height:0; }
    QScrollBar::add-page, QScrollBar::sub-page { background:transparent; }

    QPushButton { border:none; border-radius:9px; padding:0 14px; min-height:34px; font-weight:800; font-size:12px; }
    QPushButton#add  { background:#2F6FED; color:white; }
    QPushButton#add:hover  { background:#2461D6; }
    QPushButton#edit { background:#EAF2FF; color:#2F6FED; }
    QPushButton#edit:hover { background:#DCE9FF; }
    QPushButton#del  { background:white; color:#0B2A55; border:1px solid #D5E1F5; }
    QPushButton#del:hover  { background:#F4F8FF; }
    QPushButton#exp  { background:#1FA971; color:white; min-height:30px; }
    QPushButton#exp:hover  { background:#188F5F; }
    QPushButton#link { background:transparent; color:#2F6FED; min-height:20px; padding:0; font-weight:700; }
    QPushButton#link:hover { color:#2461D6; }
    QPushButton#small { background:#2F6FED; color:white; min-height:30px; }
    QPushButton#small:hover { background:#2461D6; }

    QTabWidget::pane { border:none; border-top:1px solid #E3EBF7; background:white; }
    QTabBar { font-weight:700; }
    QTabBar::tab { background:transparent; color:#6C7C98; padding:6px 18px; margin-right:6px; min-height:18px;
        border-bottom:3px solid transparent; }
    QTabBar::tab:selected { color:#2F6FED; border-bottom:3px solid #2F6FED; }

    QProgressBar { border:none; background:#E6EEFA; border-radius:6px; text-align:center;
        min-height:12px; max-height:12px; }
    QProgressBar::chunk { background:#2F6FED; border-radius:6px; }

    QListWidget { background:white; border:none; outline:0; }
    QListWidget::item { padding:6px 8px; border-bottom:1px solid #EEF3FB; }
    QListWidget::item:selected { background:#EAF2FF; color:#0B2A55; }
)CSS";

QColor couleurEtat(const QString &etat)
{
    if (etat == "Opérationnel")
        return kGreen;
    if (etat == "En maintenance")
        return kBlue;
    if (etat == "En panne")
        return kNavy;
    return QColor("#9AA8C2");
}

QString csvField(QString value)
{
    value.replace("\"", "\"\"");
    return "\"" + value + "\"";
}

// Texte sans accents ni majuscules : la recherche « operationnel » trouve « Opérationnel ».
QString normaliser(const QString &texte)
{
    const QString decompose = texte.normalized(QString::NormalizationForm_D);
    QString resultat;
    resultat.reserve(decompose.size());
    for (const QChar &c : decompose) {
        if (c.category() != QChar::Mark_NonSpacing)
            resultat.append(c);
    }
    return resultat.toLower();
}

// Ordre de tri des états : opérationnel, maintenance, panne.
int rangEtat(const QString &etat)
{
    if (etat == "Opérationnel")   return 0;
    if (etat == "En maintenance") return 1;
    if (etat == "En panne")       return 2;
    return 3;
}

QString texteNiveau(int niveau)
{
    if (niveau == Predictif::Eleve)
        return QString::fromUtf8("Élevé");
    return niveau == Predictif::Moyen ? QString("Moyen") : QString("Faible");
}

QString couleurNiveau(int niveau)
{
    if (niveau == Predictif::Eleve)
        return "#E5484D";
    return niveau == Predictif::Moyen ? QString("#2F6FED") : QString("#1FA971");
}

// Cellule triable : compare la valeur numérique (Qt::UserRole) si elle existe
// (dates, utilisation) au lieu du texte.
class SortItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem;
    bool operator<(const QTableWidgetItem &other) const override
    {
        const QVariant a = data(Qt::UserRole);
        const QVariant b = other.data(Qt::UserRole);
        if (a.isValid() && b.isValid())
            return a.toLongLong() < b.toLongLong();
        return QTableWidgetItem::operator<(other);
    }
};

QWidget *champ(const QString &label, QWidget *input)
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(4);
    auto *lb = new QLabel(label);
    lb->setObjectName("fieldLabel");
    l->addWidget(lb);
    l->addWidget(input);
    return w;
}

QLabel *titreSection(const QString &texte)
{
    auto *l = new QLabel(texte);
    l->setObjectName("section");
    return l;
}

QFrame *nouvelleCarte()
{
    auto *c = new QFrame;
    c->setObjectName("card");
    return c;
}

QLabel *petitLabel(const QString &texte)
{
    auto *l = new QLabel(texte);
    l->setObjectName("fieldLabel");
    return l;
}

QLabel *valeurDetail()
{
    auto *l = new QLabel("--");
    l->setObjectName("detailVal");
    return l;
}

} // namespace

// ---------------------------------------------------------------------------
//  Widget : anneau (donut) de répartition des états
// ---------------------------------------------------------------------------
class DonutWidget : public QWidget
{
public:
    explicit DonutWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(130, 130);
    }

    void setValeurs(int op, int maint, int panne)
    {
        m_op = op;
        m_maint = maint;
        m_panne = panne;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int total = m_op + m_maint + m_panne;
        const qreal thick = 16;
        const QRectF ring = QRectF(rect()).adjusted(thick / 2 + 2, thick / 2 + 2,
                                                    -thick / 2 - 2, -thick / 2 - 2);

        if (total == 0) {
            p.setPen(QPen(QColor("#E6EEFA"), thick));
            p.drawEllipse(ring);
        } else {
            struct Seg { int n; QColor c; };
            const Seg segs[] = {{m_op, kGreen}, {m_panne, kNavy}, {m_maint, kBlue}};
            int start = 90 * 16;
            for (const Seg &s : segs) {
                if (s.n <= 0)
                    continue;
                const int span = qRound(-360.0 * 16 * s.n / total);
                p.setPen(QPen(s.c, thick, Qt::SolidLine, Qt::FlatCap));
                p.drawArc(ring, start, span);
                start += span;
            }
        }

        p.setPen(kNavy);
        QFont f = font();
        f.setPixelSize(28);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(0, 36, width(), 36), Qt::AlignCenter, QString::number(total));

        f.setPixelSize(10);
        f.setBold(false);
        p.setFont(f);
        p.drawText(QRectF(0, 72, width(), 16), Qt::AlignCenter, QString::fromUtf8("ÉQUIPEMENTS"));
    }

private:
    int m_op = 0, m_maint = 0, m_panne = 0;
};

// ---------------------------------------------------------------------------
//  Widget : illustration d'une pompe (équivalent de la bouteille des Produits)
// ---------------------------------------------------------------------------
class MachineWidget : public QWidget
{
public:
    explicit MachineWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(105, 135);
    }

    void setEtat(const QString &etat)
    {
        m_led = etat.isEmpty() ? QColor("#B8C5DD") : couleurEtat(etat);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const qreal w = width(), h = height();

        // socle
        p.setPen(Qt::NoPen);
        p.setBrush(kNavy);
        p.drawRoundedRect(QRectF(w * 0.04, h * 0.86, w * 0.92, h * 0.07), 4, 4);

        // tuyau de sortie (vers le haut)
        p.setBrush(QColor("#1E4FB8"));
        p.drawRoundedRect(QRectF(w * 0.63, h * 0.10, w * 0.14, h * 0.30), 3, 3);
        p.setBrush(kBlue);
        p.drawRoundedRect(QRectF(w * 0.58, h * 0.07, w * 0.24, h * 0.07), 3, 3);

        // moteur
        QLinearGradient g(0, 0, w * 0.5, 0);
        g.setColorAt(0, QColor("#3C7BFF"));
        g.setColorAt(1, QColor("#1E4FB8"));
        p.setBrush(g);
        p.drawRoundedRect(QRectF(w * 0.06, h * 0.40, w * 0.40, h * 0.46), 10, 10);

        // ailettes de refroidissement
        p.setPen(QPen(QColor(255, 255, 255, 110), 2));
        for (int i = 1; i <= 4; ++i) {
            const qreal x = w * (0.06 + 0.08 * i);
            p.drawLine(QPointF(x, h * 0.46), QPointF(x, h * 0.80));
        }

        // voyant d'état
        p.setPen(QPen(Qt::white, 2));
        p.setBrush(m_led);
        p.drawEllipse(QPointF(w * 0.15, h * 0.47), 6, 6);

        // corps de pompe (volute)
        p.setPen(QPen(kBlue, 5));
        p.setBrush(QColor("#DDF0FF"));
        p.drawEllipse(QPointF(w * 0.70, h * 0.62), w * 0.25, w * 0.25);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#8FD3F4"));
        p.drawEllipse(QPointF(w * 0.70, h * 0.62), w * 0.15, w * 0.15);
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(w * 0.70, h * 0.62), w * 0.06, w * 0.06);

        // liaison moteur / pompe
        p.setBrush(kNavy);
        p.drawRect(QRectF(w * 0.46, h * 0.59, w * 0.04, h * 0.06));
    }

private:
    QColor m_led = QColor("#B8C5DD");
};

// ---------------------------------------------------------------------------
//  Construction
// ---------------------------------------------------------------------------
GestionEquipement::GestionEquipement(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("page");
    appliquerTheme();
    buildUi();
    majHorloge();
    recharger();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &GestionEquipement::majHorloge);
    m_timer->start(1000);
}

void GestionEquipement::appliquerTheme()
{
    QPalette pal;
    pal.setColor(QPalette::Window, QColor("#F6F9FF"));
    pal.setColor(QPalette::WindowText, kNavy);
    pal.setColor(QPalette::Base, Qt::white);
    pal.setColor(QPalette::AlternateBase, QColor("#F5F9FF"));
    pal.setColor(QPalette::Text, kNavy);
    pal.setColor(QPalette::Button, Qt::white);
    pal.setColor(QPalette::ButtonText, kNavy);
    pal.setColor(QPalette::Highlight, QColor("#DCE9FF"));
    pal.setColor(QPalette::HighlightedText, kNavy);
    pal.setColor(QPalette::ToolTipBase, Qt::white);
    pal.setColor(QPalette::ToolTipText, kNavy);
    pal.setColor(QPalette::PlaceholderText, QColor("#A9B5C9"));
    setPalette(pal);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(true);
    setStyleSheet(kStyle);
}

void GestionEquipement::buildUi()
{
    // Toute la page est dans une zone défilante : rien n'est écrasé sur un petit écran.
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);

    auto *content = new QWidget;
    content->setObjectName("content");
    content->setAttribute(Qt::WA_StyledBackground, true);
    scroll->setWidget(content);

    auto *root = new QVBoxLayout(content);
    root->setContentsMargins(20, 14, 16, 14);
    root->setSpacing(12);

    root->addWidget(buildEntete());

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(12);
    row1->addWidget(buildCarteDetails(), 1);
    row1->addWidget(buildCarteEtat(), 1);
    row1->addWidget(buildCarteTypes(), 1);
    root->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(12);
    row2->addWidget(buildCarteFormulaire(), 10);
    row2->addWidget(buildCarteListe(), 17);
    root->addLayout(row2, 1);
}

QWidget *GestionEquipement::buildEntete()
{
    auto *widget = new QWidget;
    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *texts = new QVBoxLayout;
    texts->setSpacing(4);
    auto *title = new QLabel(QString::fromUtf8("GESTION DES ÉQUIPEMENTS"));
    title->setObjectName("pageTitle");
    auto *sub = new QLabel("Suivi des équipements, maintenance prédictive et alertes");
    sub->setObjectName("pageSub");
    texts->addWidget(title);
    texts->addWidget(sub);
    layout->addLayout(texts, 1);

    auto *clock = new QVBoxLayout;
    m_date = new QLabel;
    m_date->setObjectName("date");
    m_date->setAlignment(Qt::AlignRight);
    m_heure = new QLabel;
    m_heure->setObjectName("heure");
    m_heure->setAlignment(Qt::AlignRight);
    clock->addWidget(m_date);
    clock->addSpacing(4);
    clock->addWidget(m_heure);
    clock->addStretch();
    layout->addLayout(clock);
    return widget;
}

QFrame *GestionEquipement::buildCarteDetails()
{
    auto *card = nouvelleCarte();
    card->setFixedHeight(205);
    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);

    auto *left = new QVBoxLayout;
    left->setSpacing(2);
    left->addWidget(titreSection("DÉTAILS DE L'ÉQUIPEMENT"));
    left->addSpacing(2);

    m_dNom = valeurDetail();
    m_dType = valeurDetail();
    m_dLoc = valeurDetail();
    m_dEtat = valeurDetail();

    left->addWidget(petitLabel("Désignation"));
    left->addWidget(m_dNom);
    left->addSpacing(3);
    left->addWidget(petitLabel("Type"));
    left->addWidget(m_dType);
    left->addSpacing(3);
    left->addWidget(petitLabel("Localisation"));
    left->addWidget(m_dLoc);
    left->addSpacing(3);
    left->addWidget(petitLabel("État"));
    left->addWidget(m_dEtat);
    left->addStretch();
    layout->addLayout(left, 1);

    m_machine = new MachineWidget;
    layout->addWidget(m_machine, 0, Qt::AlignRight | Qt::AlignTop);
    return card;
}

QFrame *GestionEquipement::buildCarteEtat()
{
    auto *card = nouvelleCarte();
    card->setFixedHeight(205);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->addWidget(titreSection("ÉTAT DES ÉQUIPEMENTS"));

    auto *row = new QHBoxLayout;
    row->setSpacing(16);
    m_donut = new DonutWidget;
    row->addWidget(m_donut, 0, Qt::AlignTop);

    auto *legend = new QVBoxLayout;
    legend->setSpacing(8);
    m_legOp = new QLabel;
    m_legMaint = new QLabel;
    m_legPanne = new QLabel;
    for (auto *l : {m_legOp, m_legMaint, m_legPanne}) {
        l->setObjectName("legend");
        l->setTextFormat(Qt::RichText);
        legend->addWidget(l);
    }
    legend->addStretch();
    row->addLayout(legend, 1);
    layout->addLayout(row);
    layout->addStretch();
    return card;
}

QFrame *GestionEquipement::buildCarteTypes()
{
    auto *card = nouvelleCarte();
    card->setFixedHeight(205);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);
    layout->addWidget(titreSection("RÉPARTITION PAR TYPE"));
    layout->addSpacing(4);

    for (int i = 0; i < 3; ++i) {
        auto *head = new QHBoxLayout;
        m_typeNom[i] = new QLabel;
        m_typeNom[i]->setObjectName("barName");
        m_typePct[i] = new QLabel;
        m_typePct[i]->setObjectName("barPct");
        m_typePct[i]->setAlignment(Qt::AlignRight);
        head->addWidget(m_typeNom[i], 1);
        head->addWidget(m_typePct[i]);
        layout->addLayout(head);

        m_typeBar[i] = new QProgressBar;
        m_typeBar[i]->setRange(0, 100);
        m_typeBar[i]->setTextVisible(false);
        layout->addWidget(m_typeBar[i]);
        layout->addSpacing(4);
    }
    layout->addStretch();
    return card;
}

QFrame *GestionEquipement::buildCarteFormulaire()
{
    auto *card = nouvelleCarte();
    card->setMinimumWidth(380);
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 14);
    layout->setSpacing(8);

    auto *head = new QHBoxLayout;
    head->addWidget(titreSection("AJOUTER UN ÉQUIPEMENT"), 1);
    auto *vider = new QPushButton("Vider le formulaire");
    vider->setObjectName("link");
    vider->setCursor(Qt::PointingHandCursor);
    head->addWidget(vider);
    layout->addLayout(head);

    m_id = new QLineEdit;
    m_id->setPlaceholderText("Automatique");
    m_id->setEnabled(false);   // calculé automatiquement par le DAO
    m_designation = new QLineEdit;
    m_designation->setPlaceholderText("Ex : Pompe centrifuge P-09");
    m_marque = new QLineEdit;
    m_marque->setPlaceholderText("Ex : Grundfos");
    m_modele = new QLineEdit;
    m_modele->setPlaceholderText("Ex : CR-45");
    m_localisation = new QLineEdit;
    m_localisation->setPlaceholderText("Ex : Atelier A");
    m_idUsine = new QLineEdit;
    m_idUsine->setPlaceholderText("Ex : 1");
    m_idUsine->setValidator(new QIntValidator(0, 999999999, m_idUsine));

    m_type = new QComboBox;
    m_type->addItems(kTypes);
    m_etat = new QComboBox;
    m_etat->addItems(kEtats);

    m_utilisation = new QSpinBox;
    m_utilisation->setRange(0, 100);
    m_utilisation->setSuffix(" %");
    m_utilisation->setButtonSymbols(QAbstractSpinBox::NoButtons);

    auto makeDate = []() {
        auto *d = new QDateEdit(QDate::currentDate());
        d->setCalendarPopup(true);
        d->setDisplayFormat(kFmt);
        return d;
    };
    m_dInstall = makeDate();
    m_dDerniere = makeDate();
    m_dProchaine = makeDate();
    m_dProchaine->setDate(QDate::currentDate().addMonths(6));

    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(6);
    grid->addWidget(champ("ID (automatique)", m_id), 0, 0);
    grid->addWidget(champ("Désignation", m_designation), 0, 1);
    grid->addWidget(champ("Type", m_type), 1, 0);
    grid->addWidget(champ("Marque", m_marque), 1, 1);
    grid->addWidget(champ("Modèle", m_modele), 2, 0);
    grid->addWidget(champ("Localisation", m_localisation), 2, 1);
    grid->addWidget(champ("État", m_etat), 3, 0);
    grid->addWidget(champ("Niveau d'utilisation", m_utilisation), 3, 1);
    grid->addWidget(champ("Date d'installation", m_dInstall), 4, 0);
    grid->addWidget(champ("Dernière maintenance", m_dDerniere), 4, 1);
    grid->addWidget(champ("Prochaine maintenance", m_dProchaine), 5, 0);
    grid->addWidget(champ("ID usine", m_idUsine), 5, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    layout->addLayout(grid);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(10);
    auto *add = new QPushButton("Ajouter");
    add->setObjectName("add");
    auto *edit = new QPushButton("Modifier");
    edit->setObjectName("edit");
    auto *del = new QPushButton("Supprimer");
    del->setObjectName("del");
    for (auto *b : {add, edit, del}) {
        b->setCursor(Qt::PointingHandCursor);
        buttons->addWidget(b, 1);
    }
    layout->addSpacing(6);
    layout->addLayout(buttons);
    layout->addStretch();

    connect(add, &QPushButton::clicked, this, &GestionEquipement::onAjouter);
    connect(edit, &QPushButton::clicked, this, &GestionEquipement::onModifier);
    connect(del, &QPushButton::clicked, this, &GestionEquipement::onSupprimer);
    connect(vider, &QPushButton::clicked, this, [this]() {
        m_table->clearSelection();
        viderFormulaire();
        m_id->setFocus();
    });
    return card;
}

QFrame *GestionEquipement::buildCarteListe()
{
    auto *card = nouvelleCarte();
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 8);
    layout->setSpacing(10);

    // ligne 1 : titre + export  /  ligne 2 : recherche + filtres
    auto *bar = new QHBoxLayout;
    bar->addWidget(titreSection("LISTE DES ÉQUIPEMENTS"));
    bar->addStretch(1);

    auto *statsButton = new QPushButton("Statistiques");
    statsButton->setObjectName("small");
    statsButton->setCursor(Qt::PointingHandCursor);
    bar->addWidget(statsButton);

    auto *exportButton = new QPushButton("Exporter Excel");
    exportButton->setObjectName("exp");
    exportButton->setCursor(Qt::PointingHandCursor);
    bar->addWidget(exportButton);
    layout->addLayout(bar);

    m_recherche = new QLineEdit;
    m_recherche->setPlaceholderText("Rechercher par ID, nom, type, localisation ou état...");
    m_recherche->setClearButtonEnabled(true);

    m_filtreType = new QComboBox;
    m_filtreType->addItem("Tous les types");
    m_filtreType->addItems(kTypes);

    m_filtreEtat = new QComboBox;
    m_filtreEtat->addItem("Tous les états");
    m_filtreEtat->addItems(kEtats);

    auto *filtres = new QHBoxLayout;
    filtres->setSpacing(8);
    filtres->addWidget(m_recherche, 3);
    filtres->addWidget(m_filtreType, 2);
    filtres->addWidget(m_filtreEtat, 2);
    layout->addLayout(filtres);

    // tri : état, type, date d'installation, niveau d'utilisation (et autres)
    m_triCombo = new QComboBox;
    m_triCombo->addItem("ID", int(cID));
    m_triCombo->addItem("Désignation", int(cDesignation));
    m_triCombo->addItem("Type", int(cType));
    m_triCombo->addItem("État", int(cEtat));
    m_triCombo->addItem("Date d'installation", int(cInstall));
    m_triCombo->addItem("Niveau d'utilisation", int(cUtilisation));
    m_triCombo->addItem("Prochaine maintenance", int(cProchaine));

    m_triOrdreBtn = new QPushButton;
    m_triOrdreBtn->setObjectName("edit");
    m_triOrdreBtn->setCursor(Qt::PointingHandCursor);
    majBoutonOrdre();

    auto *triLayout = new QHBoxLayout;
    triLayout->setSpacing(8);
    triLayout->addWidget(petitLabel("Trier par"));
    triLayout->addWidget(m_triCombo, 2);
    triLayout->addWidget(m_triOrdreBtn, 1);
    triLayout->addStretch(2);
    layout->addLayout(triLayout);

    // tableau
    m_table = new QTableWidget(0, cCount);
    m_table->setHorizontalHeaderLabels({
        "ID", "Désignation", "Type", "Marque", "Modèle", "Localisation",
        "Installation", "État", "Util.", "Dernière maint.", "Proch. maint.", "ID usine"
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(false);
    m_table->setFrameShape(QFrame::NoFrame);
    m_table->setTextElideMode(Qt::ElideRight);
    m_table->setWordWrap(false);
    m_table->setMinimumHeight(150);
    m_table->horizontalHeader()->setMinimumSectionSize(40);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(30);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(cDesignation, QHeaderView::Stretch);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setColumnWidth(cID, 55);
    m_table->setColumnWidth(cType, 95);
    m_table->setColumnWidth(cLocalisation, 110);
    m_table->setColumnWidth(cEtat, 100);
    m_table->setColumnWidth(cUtilisation, 55);
    m_table->setColumnWidth(cProchaine, 90);
    m_table->setColumnWidth(cIdUsine, 75);
    m_table->setColumnWidth(cMarque, 100);
    m_table->setColumnWidth(cModele, 100);
    m_table->setColumnWidth(cInstall, 95);
    m_table->setColumnWidth(cDerniere, 115);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_table->setSortingEnabled(true);
    m_table->sortByColumn(cID, Qt::AscendingOrder);
    layout->addWidget(m_table, 1);

    layout->addWidget(buildOnglets());

    connect(exportButton, &QPushButton::clicked, this, &GestionEquipement::onExporter);
    connect(statsButton, &QPushButton::clicked, this, &GestionEquipement::onStatistiques);
    connect(m_triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { onTri(); });
    connect(m_triOrdreBtn, &QPushButton::clicked, this, [this]() {
        m_triOrdre = (m_triOrdre == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
        onTri();
    });
    // un clic sur l'en-tête d'une colonne trie aussi : on garde la liste déroulante synchronisée
    connect(m_table->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
            [this](int col, Qt::SortOrder ordre) {
                m_triCol = col;
                m_triOrdre = ordre;
                const int index = m_triCombo->findData(col);
                if (index >= 0) {
                    QSignalBlocker bloc(m_triCombo);
                    m_triCombo->setCurrentIndex(index);
                }
                majBoutonOrdre();
            });
    connect(m_recherche, &QLineEdit::textChanged, this, &GestionEquipement::appliquerFiltre);
    connect(m_filtreType, &QComboBox::currentTextChanged, this, &GestionEquipement::appliquerFiltre);
    connect(m_filtreEtat, &QComboBox::currentTextChanged, this, &GestionEquipement::appliquerFiltre);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &GestionEquipement::onSelection);
    return card;
}

QWidget *GestionEquipement::buildOnglets()
{
    m_onglets = new QTabWidget;
    m_onglets->setFixedHeight(255);

    // --- Maintenance prédictive IA ---
    auto *ia = new QWidget;
    auto *iaRoot = new QHBoxLayout(ia);
    iaRoot->setContentsMargins(4, 6, 4, 2);
    iaRoot->setSpacing(14);

    auto *g = new QGridLayout;
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(3);

    m_iaEquip = new QComboBox;
    m_iaScore = new QProgressBar;
    m_iaScore->setRange(0, 100);
    m_iaScore->setValue(0);
    m_iaScore->setTextVisible(false);
    m_iaNiveau = new QLabel("-");
    m_iaNiveau->setObjectName("iaValue");
    m_iaDate = new QLabel("-");
    m_iaDate->setObjectName("iaValue");
    m_iaPanne = new QLabel("-");
    m_iaPanne->setObjectName("iaValue");
    m_iaMotifs = new QLabel("-");
    m_iaMotifs->setWordWrap(true);
    m_iaMotifs->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto *analyse = new QPushButton("Analyser");
    analyse->setObjectName("small");
    analyse->setCursor(Qt::PointingHandCursor);
    m_iaAppliquer = new QPushButton("Appliquer la date");
    m_iaAppliquer->setObjectName("small");
    m_iaAppliquer->setCursor(Qt::PointingHandCursor);
    m_iaAppliquer->setEnabled(false);
    auto *historique = new QPushButton("Historique");
    historique->setObjectName("small");
    historique->setCursor(Qt::PointingHandCursor);

    g->addWidget(petitLabel("Équipement"), 0, 0);
    g->addWidget(m_iaEquip, 0, 1);
    g->addWidget(analyse, 0, 2);
    g->addWidget(petitLabel("Score de risque"), 1, 0);
    g->addWidget(m_iaScore, 1, 1, 1, 2);
    g->addWidget(petitLabel("Niveau de risque"), 2, 0);
    g->addWidget(m_iaNiveau, 2, 1, 1, 2);
    g->addWidget(petitLabel("Maintenance conseillée"), 3, 0);
    g->addWidget(m_iaDate, 3, 1);
    g->addWidget(m_iaAppliquer, 3, 2);
    g->addWidget(petitLabel("Prochaine panne estimée"), 4, 0);
    g->addWidget(m_iaPanne, 4, 1);
    g->addWidget(historique, 4, 2);
    g->addWidget(petitLabel("Motifs"), 5, 0, Qt::AlignTop);
    g->addWidget(m_iaMotifs, 5, 1, 1, 2);
    g->setColumnStretch(1, 1);
    g->setRowStretch(6, 1);
    iaRoot->addLayout(g, 3);

    auto *droite = new QVBoxLayout;
    droite->setSpacing(4);
    droite->addWidget(petitLabel("Équipements à risque (cliquer pour analyser)"));
    m_iaClassement = new QListWidget;
    droite->addWidget(m_iaClassement, 1);
    iaRoot->addLayout(droite, 2);

    connect(analyse, &QPushButton::clicked, this, &GestionEquipement::onAnalyser);
    connect(historique, &QPushButton::clicked, this, &GestionEquipement::onHistorique);
    connect(m_iaAppliquer, &QPushButton::clicked, this, &GestionEquipement::onAppliquerDate);
    connect(m_iaEquip, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { onAnalyser(); });
    connect(m_iaClassement, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        const int index = m_iaEquip->findData(item->data(Qt::UserRole));
        if (index >= 0)
            m_iaEquip->setCurrentIndex(index);
    });
    m_onglets->addTab(ia, "Maintenance IA");

    // --- Smart Alert ---
    m_alertes = new QListWidget;
    m_onglets->addTab(m_alertes, "Smart Alert");
    return m_onglets;
}

// ---------------------------------------------------------------------------
//  Données
// ---------------------------------------------------------------------------
void GestionEquipement::signalerErreur(const QString &titre, const QString &detail)
{
    QMessageBox::critical(this, titre,
                          "Erreur de base de données :\n\n" + detail);
}

// Relit toute la table EQUIPEMENT depuis Oracle et reconstruit le tableau.
void GestionEquipement::recharger(qlonglong idASelectionner)
{
    QList<Equipement> liste;
    QString erreur;
    const bool ok = EquipementDao::lister(liste, erreur);
    m_equipements = liste;

    {
        QSignalBlocker bloc(m_table);
        m_table->setSortingEnabled(false);
        m_table->clearSelection();
        m_table->setRowCount(0);

        for (const Equipement &e : liste) {
            const int row = m_table->rowCount();
            m_table->insertRow(row);
            auto d = [](const QDate &x) { return x.isValid() ? x.toString(kFmt) : QString(); };
            ecrireLigne(row, {QString::number(e.id), e.designation, e.type, e.marque, e.modele,
                              e.localisation, d(e.install), e.etat, QString::number(e.utilisation),
                              d(e.derniere), d(e.prochaine),
                              e.idUsine > 0 ? QString::number(e.idUsine) : QString()});
        }
        m_table->setSortingEnabled(true);
        m_table->sortByColumn(m_triCol, m_triOrdre);   // on conserve le tri choisi
    }

    chargerEvenements();
    rafraichirTout();

    int row = idASelectionner >= 0 ? trouverLigneParID(QString::number(idASelectionner)) : -1;
    if (row >= 0)
        m_table->selectRow(row);      // déclenche onSelection()
    else
        afficherDetails(-1);

    if (!ok)
        signalerErreur("Chargement des équipements", erreur);
}

// Écrit une ligne complète. Le tri doit être désactivé par l'appelant.
void GestionEquipement::ecrireLigne(int row, const QStringList &v)
{
    if (v.size() < cCount)
        return;

    for (int col = 0; col < cCount; ++col) {
        QString value = v.at(col);
        auto *item = new SortItem;

        if (col == cID) {
            item->setData(Qt::UserRole, value.toLongLong());
            item->setText(value);
        } else if (col == cIdUsine) {
            item->setData(Qt::UserRole, value.toLongLong());
            item->setText(value);
            item->setTextAlignment(Qt::AlignCenter);
        } else if (col == cUtilisation) {
            item->setData(Qt::UserRole, value.remove('%').trimmed().toInt());
            item->setText(value.trimmed() + " %");
            item->setTextAlignment(Qt::AlignCenter);
        } else if (col == cInstall || col == cDerniere || col == cProchaine) {
            item->setText(value);
            item->setData(Qt::UserRole, qint64(QDate::fromString(value, kFmt).toJulianDay()));
            item->setTextAlignment(Qt::AlignCenter);
        } else if (col == cEtat) {
            item->setText(value);
            item->setData(Qt::UserRole, rangEtat(value));
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setTextAlignment(Qt::AlignCenter);
            if (value == "Opérationnel") {
                item->setForeground(QColor("#14865A"));
                item->setBackground(QColor("#E4F6EE"));
            } else if (value == "En maintenance") {
                item->setForeground(kBlue);
                item->setBackground(QColor("#E6EFFF"));
            } else {
                item->setForeground(Qt::white);
                item->setBackground(kNavy);
            }
        } else {
            item->setText(value);
        }
        m_table->setItem(row, col, item);
    }
}

QString GestionEquipement::texte(int row, int col) const
{
    QTableWidgetItem *item = m_table->item(row, col);
    return item ? item->text() : QString();
}

int GestionEquipement::trouverLigneParID(const QString &id) const
{
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (texte(row, cID).compare(id, Qt::CaseInsensitive) == 0)
            return row;
    }
    return -1;
}

// ---------------------------------------------------------------------------
//  Formulaire
// ---------------------------------------------------------------------------
void GestionEquipement::viderFormulaire()
{
    m_id->clear();
    m_designation->clear();
    m_marque->clear();
    m_modele->clear();
    m_localisation->clear();
    m_idUsine->clear();
    m_type->setCurrentIndex(0);
    m_etat->setCurrentIndex(0);
    m_utilisation->setValue(0);
    m_dInstall->setDate(QDate::currentDate());
    m_dDerniere->setDate(QDate::currentDate());
    m_dProchaine->setDate(QDate::currentDate().addMonths(6));
}

void GestionEquipement::remplirFormulaire(int row)
{
    m_id->setText(texte(row, cID));
    m_designation->setText(texte(row, cDesignation));
    m_type->setCurrentText(texte(row, cType));
    m_marque->setText(texte(row, cMarque));
    m_modele->setText(texte(row, cModele));
    m_localisation->setText(texte(row, cLocalisation));
    m_idUsine->setText(texte(row, cIdUsine));
    m_etat->setCurrentText(texte(row, cEtat));
    m_utilisation->setValue(texte(row, cUtilisation).remove('%').trimmed().toInt());

    auto setDate = [](QDateEdit *w, const QString &t) {
        const QDate d = QDate::fromString(t, kFmt);
        if (d.isValid())
            w->setDate(d);
    };
    setDate(m_dInstall, texte(row, cInstall));
    setDate(m_dDerniere, texte(row, cDerniere));
    setDate(m_dProchaine, texte(row, cProchaine));
}

// Valide le formulaire et remplit l'objet Equipement.
bool GestionEquipement::lireFormulaire(Equipement &e)
{
    if (m_designation->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champ obligatoire", "La désignation est obligatoire.");
        m_designation->setFocus();
        return false;
    }
    if (m_dProchaine->date() < m_dDerniere->date()) {
        QMessageBox::warning(this, "Dates incohérentes",
                             "La prochaine maintenance doit être postérieure à la dernière.");
        return false;
    }

    e.designation  = m_designation->text().trimmed();
    e.type         = m_type->currentText();
    e.marque       = m_marque->text().trimmed();
    e.modele       = m_modele->text().trimmed();
    e.localisation = m_localisation->text().trimmed();
    e.idUsine      = m_idUsine->text().trimmed().toLongLong();
    e.etat         = m_etat->currentText();
    e.utilisation  = m_utilisation->value();
    e.install      = m_dInstall->date();
    e.derniere     = m_dDerniere->date();
    e.prochaine    = m_dProchaine->date();
    return true;
}

// ---------------------------------------------------------------------------
//  Actions (CRUD vers Oracle)
// ---------------------------------------------------------------------------
void GestionEquipement::onSelection()
{
    const int row = m_table->selectedItems().isEmpty() ? -1 : m_table->currentRow();
    if (row >= 0) {
        remplirFormulaire(row);
        const int index = m_iaEquip->findData(texte(row, cID));
        if (index >= 0)
            m_iaEquip->setCurrentIndex(index);
    }
    afficherDetails(row);
}

void GestionEquipement::onAjouter()
{
    Equipement e;
    if (!lireFormulaire(e))
        return;

    QString erreur;
    qlonglong nouvelId = 0;
    if (!EquipementDao::ajouter(e, erreur, &nouvelId)) {
        signalerErreur("Ajout impossible", erreur);
        return;
    }
    e.id = nouvelId;
    journaliserChangement(nullptr, e);
    viderFormulaire();
    recharger();
}

void GestionEquipement::onModifier()
{
    if (m_table->selectedItems().isEmpty()) {
        QMessageBox::information(this, "Modifier",
                                 "Sélectionnez d'abord un équipement dans la liste.");
        return;
    }

    const Equipement ancien = equipementDepuisLigne(m_table->currentRow());

    Equipement e;
    if (!lireFormulaire(e))
        return;
    e.id = ancien.id;

    QString erreur;
    if (!EquipementDao::modifier(e, erreur)) {
        signalerErreur("Modification impossible", erreur);
        return;
    }
    journaliserChangement(&ancien, e);
    recharger(e.id);
}

void GestionEquipement::onSupprimer()
{
    if (m_table->selectedItems().isEmpty()) {
        QMessageBox::information(this, "Supprimer", "Sélectionnez un équipement.");
        return;
    }

    const int row = m_table->currentRow();
    const qlonglong id = texte(row, cID).toLongLong();
    const QString nom = texte(row, cDesignation);

    if (QMessageBox::question(this, "Confirmation",
                              QString("Supprimer l'équipement %1 - %2 ?").arg(id).arg(nom),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
        return;

    QString erreur;
    if (!EquipementDao::supprimer(id, erreur)) {
        signalerErreur("Suppression impossible", erreur);
        return;
    }
    viderFormulaire();
    recharger();
}

void GestionEquipement::onTri()
{
    m_triCol = m_triCombo->currentData().toInt();
    majBoutonOrdre();
    m_table->sortByColumn(m_triCol, m_triOrdre);
}

void GestionEquipement::majBoutonOrdre()
{
    m_triOrdreBtn->setText(m_triOrdre == Qt::AscendingOrder ? QString::fromUtf8("↑ Croissant")
                                                            : QString::fromUtf8("↓ Décroissant"));
}

void GestionEquipement::onExporter()
{
    QString filtreChoisi;
    QString path = QFileDialog::getSaveFileName(
        this, "Exporter les équipements", "equipements.xlsx",
        "Classeur Excel (*.xlsx);;Fichier CSV (*.csv)", &filtreChoisi);
    if (path.isEmpty())
        return;

    const bool csv = path.endsWith(".csv", Qt::CaseInsensitive)
                     || (!path.endsWith(".xlsx", Qt::CaseInsensitive)
                         && filtreChoisi.contains("csv", Qt::CaseInsensitive));
    if (csv && !path.endsWith(".csv", Qt::CaseInsensitive))
        path += ".csv";
    if (!csv && !path.endsWith(".xlsx", Qt::CaseInsensitive))
        path += ".xlsx";

    // On exporte la liste affichée (filtres et tri appliqués).
    QList<int> lignes;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        if (!m_table->isRowHidden(row))
            lignes.append(row);
    }

    QString erreur;
    const bool ok = csv ? exporterCsv(path, lignes, erreur) : exporterXlsx(path, lignes, erreur);
    if (!ok) {
        QMessageBox::warning(this, "Erreur d'export", erreur);
        return;
    }
    QMessageBox::information(this, "Export réussi",
                             QString("%1 équipement(s) exporté(s) au format %2.")
                                 .arg(lignes.size()).arg(csv ? QString("CSV") : QString("Excel")));
}

bool GestionEquipement::exporterCsv(const QString &chemin, const QList<int> &lignes, QString &erreur) const
{
    QFile file(chemin);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        erreur = "Impossible de créer le fichier. Vérifiez les permissions (fichier ouvert dans Excel ?).";
        return false;
    }

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#endif
    out.setGenerateByteOrderMark(true);   // Excel lit correctement les accents

    QStringList headers;
    for (int col = 0; col < cCount; ++col) {
        QTableWidgetItem *h = m_table->horizontalHeaderItem(col);
        headers << csvField(h ? h->text() : QString());
    }
    out << headers.join(';') << '\n';

    for (int row : lignes) {
        QStringList values;
        for (int col = 0; col < cCount; ++col)
            values << csvField(texte(row, col));
        out << values.join(';') << '\n';
    }
    file.close();
    return true;
}

bool GestionEquipement::exporterXlsx(const QString &chemin, const QList<int> &lignes, QString &erreur) const
{
    using namespace Xlsx;
    auto s8 = [](const QString &t) { return t.toUtf8().toStdString(); };

    // --- feuille 1 : la liste des équipements
    Sheet liste;
    liste.name = s8(QString::fromUtf8("Équipements"));
    liste.widths = {7, 30, 16, 16, 16, 18, 14, 16, 10, 18, 18, 12};

    std::vector<Cell> entete;
    for (int col = 0; col < cCount; ++col) {
        QTableWidgetItem *h = m_table->horizontalHeaderItem(col);
        QString titre = h ? h->text() : QString();
        if (col == cUtilisation)
            titre = "Util. (%)";
        entete.push_back(Cell::head(s8(titre)));
    }
    liste.rows.push_back(entete);

    for (int row : lignes) {
        std::vector<Cell> ligne;
        for (int col = 0; col < cCount; ++col) {
            const QString t = texte(row, col);
            if (col == cID)
                ligne.push_back(Cell::num(double(t.toLongLong())));
            else if (col == cIdUsine)
                ligne.push_back(t.isEmpty() ? Cell::str(std::string()) : Cell::num(double(t.toLongLong())));
            else if (col == cUtilisation)
                ligne.push_back(Cell::num(QString(t).remove('%').trimmed().toInt()));
            else
                ligne.push_back(Cell::str(s8(t)));
        }
        liste.rows.push_back(ligne);
    }

    // --- feuille 2 : statistiques
    const StatsEquipements st = calculerStats(m_equipements, m_evenements, m_historiqueOk, m_historiqueErreur);
    Sheet stats;
    stats.name = "Statistiques";
    stats.widths = {38, 16};
    auto ligneStat = [&s8](const QString &nom, double valeur) {
        return std::vector<Cell>{Cell::str(s8(nom)), Cell::num(valeur)};
    };
    stats.rows.push_back({Cell::titre(s8(QString::fromUtf8("Statistiques des équipements")))});
    stats.rows.push_back({Cell::str(s8("Généré le " + QDate::currentDate().toString(kFmt)))});
    stats.rows.push_back({});
    stats.rows.push_back({Cell::head(s8("Indicateur")), Cell::head(s8("Valeur"))});
    stats.rows.push_back(ligneStat("Total des équipements", st.total));
    stats.rows.push_back(ligneStat("Équipements opérationnels", st.operationnels));
    stats.rows.push_back(ligneStat("Équipements en maintenance", st.enMaintenance));
    stats.rows.push_back(ligneStat("Équipements en panne", st.enPanne));
    stats.rows.push_back(ligneStat("Taux de disponibilité (%)", qRound(st.disponibilite * 10) / 10.0));
    if (st.historiqueDisponible) {
        stats.rows.push_back(ligneStat("Pannes sur les 12 derniers mois", st.totalPannes12Mois));
        stats.rows.push_back(ligneStat("Moyenne de pannes par mois", qRound(st.moyennePannesParMois * 10) / 10.0));
        stats.rows.push_back({});
        stats.rows.push_back({Cell::head(s8("Fréquence des pannes (mois)")), Cell::head(s8("Pannes"))});
        for (int i = 0; i < st.moisLabels.size(); ++i)
            stats.rows.push_back(ligneStat(st.moisLabels.at(i), st.pannesParMois.value(i)));
        stats.rows.push_back({});
        stats.rows.push_back({Cell::head(s8("Équipements les plus touchés")), Cell::head(s8("Pannes"))});
        for (const auto &p : st.pannesParEquipement)
            stats.rows.push_back(ligneStat(p.first, p.second));
    } else {
        stats.rows.push_back({});
        stats.rows.push_back({Cell::str(s8("Historique des pannes indisponible (exécutez creation_historique.sql)."))});
    }
    stats.rows.push_back({});
    stats.rows.push_back({Cell::head(s8("Type d'équipement")), Cell::head(s8("Nombre"))});
    for (const auto &p : st.parType)
        stats.rows.push_back(ligneStat(p.first, p.second));

    // --- feuille 3 : maintenance prédictive
    Sheet ia;
    ia.name = s8(QString::fromUtf8("Maintenance prédictive"));
    ia.widths = {7, 30, 16, 10, 12, 20, 22, 60};
    ia.rows.push_back({Cell::head(s8("ID")), Cell::head(s8("Désignation")), Cell::head(s8("État")),
                       Cell::head(s8("Score")), Cell::head(s8("Niveau")),
                       Cell::head(s8("Panne estimée")), Cell::head(s8("Maintenance conseillée")),
                       Cell::head(s8("Motif principal"))});
    for (int row : lignes) {
        const Predictif::Resultat r = analyserLigne(row);
        ia.rows.push_back({Cell::num(double(texte(row, cID).toLongLong())),
                           Cell::str(s8(texte(row, cDesignation))),
                           Cell::str(s8(texte(row, cEtat))),
                           Cell::num(r.score),
                           Cell::str(s8(texteNiveau(r.niveau))),
                           Cell::str(r.jourPanneEstimee > 0
                                         ? s8(QDate::fromJulianDay(r.jourPanneEstimee).toString(kFmt))
                                         : std::string("-")),
                           Cell::str(s8(QDate::fromJulianDay(r.jourMaintenance).toString(kFmt))),
                           Cell::str(r.motifs.empty() ? std::string("-") : r.motifs.front())});
    }

    const std::string donnees = build({liste, stats, ia});
    QFile file(chemin);
    if (!file.open(QIODevice::WriteOnly)) {
        erreur = "Impossible de créer le fichier. Vérifiez les permissions (fichier ouvert dans Excel ?).";
        return false;
    }
    const qint64 ecrit = file.write(donnees.data(), qint64(donnees.size()));
    file.close();
    if (ecrit != qint64(donnees.size())) {
        erreur = "Écriture incomplète du fichier Excel.";
        return false;
    }
    return true;
}

void GestionEquipement::onStatistiques()
{
    chargerEvenements();    // données à jour
    StatistiquesDialog dlg(calculerStats(m_equipements, m_evenements, m_historiqueOk, m_historiqueErreur), this);
    dlg.exec();
}

// ---------------------------------------------------------------------------
//  Maintenance prédictive (analyse de l'historique des pannes et maintenances)
// ---------------------------------------------------------------------------
// Prépare les données d'un équipement (ligne du tableau + historique) pour l'analyse.
Predictif::Resultat GestionEquipement::analyserLigne(int row) const
{
    using namespace Predictif;
    Entree in;
    in.aujourdhui = int(QDate::currentDate().toJulianDay());

    const QString etat = texte(row, cEtat);
    in.etat = etat == "En panne" ? EnPanne : (etat == "En maintenance" ? EnMaintenance : Operationnel);
    in.utilisation = texte(row, cUtilisation).remove('%').trimmed().toInt();

    auto jour = [](const QString &t) {
        const QDate d = QDate::fromString(t, kFmt);
        return d.isValid() ? int(d.toJulianDay()) : 0;
    };
    in.jourInstallation = jour(texte(row, cInstall));
    in.jourDerniereMaint = jour(texte(row, cDerniere));
    in.jourProchaineMaint = jour(texte(row, cProchaine));

    const qlonglong id = texte(row, cID).toLongLong();
    for (const Evenement &ev : m_evenements) {
        if (ev.idEquipement != id || !ev.date.isValid())
            continue;
        if (ev.type == "Panne")
            in.pannes.push_back(int(ev.date.toJulianDay()));
        else
            in.maintenances.push_back(int(ev.date.toJulianDay()));
    }
    return analyser(in);
}

void GestionEquipement::onAnalyser()
{
    m_iaJourProposee = 0;
    m_iaAppliquer->setEnabled(false);

    const int row = trouverLigneParID(m_iaEquip->currentData().toString());
    if (row < 0) {
        m_iaScore->setValue(0);
        m_iaNiveau->setText("Aucun équipement");
        m_iaNiveau->setStyleSheet("");
        m_iaDate->setText("-");
        m_iaPanne->setText("-");
        m_iaMotifs->setText("-");
        return;
    }

    const Predictif::Resultat r = analyserLigne(row);
    m_iaScore->setValue(r.score);
    m_iaNiveau->setText(texteNiveau(r.niveau) + QString(" (%1/100)").arg(r.score));
    m_iaNiveau->setStyleSheet("color:" + couleurNiveau(r.niveau) + ";");

    const QDate aujourdhui = QDate::currentDate();
    const QDate conseillee = QDate::fromJulianDay(r.jourMaintenance);
    const int delai = int(aujourdhui.daysTo(conseillee));
    m_iaDate->setText(conseillee.toString(kFmt)
                      + (delai <= 0 ? QString::fromUtf8("  (immédiat)") : QString("  (dans %1 j)").arg(delai)));
    m_iaJourProposee = r.jourMaintenance;
    m_iaAppliquer->setEnabled(true);

    if (r.jourPanneEstimee > 0) {
        m_iaPanne->setText(QDate::fromJulianDay(r.jourPanneEstimee).toString(kFmt)
                           + QString("  (MTBF %1 j)").arg(r.mtbfJours));
    } else {
        m_iaPanne->setText(QString::fromUtf8("Données insuffisantes (2 pannes minimum)"));
    }

    QStringList lignes;
    for (int i = 0; i < int(r.motifs.size()) && i < 3; ++i)
        lignes << QString::fromUtf8("• ") + QString::fromUtf8(r.motifs[size_t(i)].c_str());
    if (!m_historiqueOk)
        lignes << QString::fromUtf8("• Historique indisponible : exécutez creation_historique.sql");
    m_iaMotifs->setText(lignes.join("\n"));
}

// Applique la date de maintenance préventive proposée comme « prochaine maintenance ».
void GestionEquipement::onAppliquerDate()
{
    const int row = trouverLigneParID(m_iaEquip->currentData().toString());
    if (row < 0 || m_iaJourProposee <= 0)
        return;

    Equipement e = equipementDepuisLigne(row);
    const QDate nouvelle = QDate::fromJulianDay(m_iaJourProposee);
    if (QMessageBox::question(this, "Maintenance préventive",
                              QString("Planifier la prochaine maintenance de « %1 » le %2 ?")
                                  .arg(e.designation, nouvelle.toString(kFmt)),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
        return;

    e.prochaine = nouvelle;
    QString erreur;
    if (!EquipementDao::modifier(e, erreur)) {
        signalerErreur("Modification impossible", erreur);
        return;
    }
    recharger(e.id);
}

void GestionEquipement::onHistorique()
{
    const int row = trouverLigneParID(m_iaEquip->currentData().toString());
    if (row < 0) {
        QMessageBox::information(this, "Historique", "Sélectionnez d'abord un équipement.");
        return;
    }
    HistoriqueDialog dlg(texte(row, cID).toLongLong(), texte(row, cDesignation), this);
    dlg.exec();

    chargerEvenements();
    rafraichirTout();
}

void GestionEquipement::chargerEvenements()
{
    QString erreur;
    m_historiqueOk = EquipementDao::listerEvenements(m_evenements, erreur);
    m_historiqueErreur = m_historiqueOk ? QString() : erreur;
    if (!m_historiqueOk)
        m_evenements.clear();
}

Equipement GestionEquipement::equipementDepuisLigne(int row) const
{
    Equipement e;
    e.id           = texte(row, cID).toLongLong();
    e.designation  = texte(row, cDesignation);
    e.type         = texte(row, cType);
    e.marque       = texte(row, cMarque);
    e.modele       = texte(row, cModele);
    e.localisation = texte(row, cLocalisation);
    e.idUsine      = texte(row, cIdUsine).toLongLong();
    e.etat         = texte(row, cEtat);
    e.utilisation  = texte(row, cUtilisation).remove('%').trimmed().toInt();
    e.install      = QDate::fromString(texte(row, cInstall), kFmt);
    e.derniere     = QDate::fromString(texte(row, cDerniere), kFmt);
    e.prochaine    = QDate::fromString(texte(row, cProchaine), kFmt);
    return e;
}

// Alimente l'historique automatiquement : passage en panne, en maintenance,
// ou mise à jour de la date de dernière maintenance. Les erreurs sont ignorées
// (la table d'historique peut ne pas encore exister).
void GestionEquipement::journaliserChangement(const Equipement *ancien, const Equipement &nv)
{
    const QDate aujourdhui = QDate::currentDate();
    auto enregistrer = [&nv](const QString &type, const QDate &date, const QString &description) {
        Evenement ev;
        ev.idEquipement = nv.id;
        ev.type = type;
        ev.date = date;
        ev.description = description;
        QString erreur;
        EquipementDao::ajouterEvenement(ev, erreur, true);
    };

    if (nv.etat == "En panne" && (!ancien || ancien->etat != "En panne"))
        enregistrer("Panne", aujourdhui, "Panne déclarée (changement d'état)");
    if (nv.etat == "En maintenance" && (!ancien || ancien->etat != "En maintenance"))
        enregistrer("Maintenance", aujourdhui, "Mise en maintenance");
    if (ancien && nv.derniere.isValid() && nv.derniere != ancien->derniere && nv.derniere <= aujourdhui)
        enregistrer("Maintenance", nv.derniere, "Maintenance effectuée (date de dernière maintenance mise à jour)");
}

// ---------------------------------------------------------------------------
//  Rafraîchissement de l'affichage
// ---------------------------------------------------------------------------
void GestionEquipement::majHorloge()
{
    const QLocale fr(QLocale::French);
    QString d = fr.toString(QDate::currentDate(), "dddd dd MMMM yyyy");
    if (!d.isEmpty())
        d[0] = d.at(0).toUpper();
    m_date->setText(d);
    m_heure->setText(QTime::currentTime().toString("HH:mm:ss"));
}

void GestionEquipement::afficherDetails(int row)
{
    if (row < 0 || row >= m_table->rowCount()) {
        m_dNom->setText("Aucune sélection");
        m_dType->setText("--");
        m_dLoc->setText("--");
        m_dEtat->setText("--");
        m_dEtat->setStyleSheet("");
        m_machine->setEtat(QString());
        return;
    }

    const QString etat = texte(row, cEtat);
    m_dNom->setText(texte(row, cDesignation));
    m_dType->setText(texte(row, cType));
    m_dLoc->setText(texte(row, cLocalisation));
    m_dEtat->setText(etat + "  ·  " + texte(row, cUtilisation));
    m_dEtat->setStyleSheet("color:" + couleurEtat(etat).name() + ";");
    m_machine->setEtat(etat);
}

void GestionEquipement::rafraichirResume()
{
    const int total = m_table->rowCount();
    int op = 0, panne = 0, maint = 0;
    QMap<QString, int> parType;

    for (int row = 0; row < total; ++row) {
        const QString etat = texte(row, cEtat);
        if (etat == "Opérationnel")
            ++op;
        else if (etat == "En panne")
            ++panne;
        else
            ++maint;
        parType[texte(row, cType)]++;
    }

    m_donut->setValeurs(op, maint, panne);

    auto legende = [](const QString &titre, int n, const QString &couleur) {
        return QString("<span style='color:%1;'>●&nbsp;&nbsp;%2 : %3</span>").arg(couleur, titre).arg(n);
    };
    m_legOp->setText(legende("Opérationnels", op, "#1FA971"));
    m_legMaint->setText(legende("En maintenance", maint, "#2F6FED"));
    m_legPanne->setText(legende("En panne", panne, "#0B2A55"));

    // 3 types les plus nombreux
    QList<QPair<int, QString>> tri;
    for (auto it = parType.constBegin(); it != parType.constEnd(); ++it)
        tri.append({it.value(), it.key()});
    std::sort(tri.begin(), tri.end(), [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
        return a.first != b.first ? a.first > b.first : a.second < b.second;
    });

    for (int i = 0; i < 3; ++i) {
        const bool visible = i < tri.size() && total > 0;
        m_typeNom[i]->setVisible(visible);
        m_typePct[i]->setVisible(visible);
        m_typeBar[i]->setVisible(visible);
        if (!visible)
            continue;
        const int pct = qRound(100.0 * tri.at(i).first / total);
        m_typeNom[i]->setText(tri.at(i).second);
        m_typePct[i]->setText(QString::number(pct) + "%");
        m_typeBar[i]->setValue(pct);
    }
}

void GestionEquipement::rafraichirListeIA()
{
    const QString precedent = m_iaEquip->currentData().toString();
    QSignalBlocker bloc(m_iaEquip);
    m_iaEquip->clear();
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QString id = texte(row, cID);
        m_iaEquip->addItem(id + " - " + texte(row, cDesignation), id);
    }
    const int index = m_iaEquip->findData(precedent);
    if (index >= 0)
        m_iaEquip->setCurrentIndex(index);
}

void GestionEquipement::rafraichirAlertes()
{
    m_alertes->clear();
    const QDate today = QDate::currentDate();

    auto ajouter = [this](const QString &msg, const QColor &c, bool gras) {
        auto *item = new QListWidgetItem("●  " + msg);
        item->setForeground(c);
        if (gras) {
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
        m_alertes->addItem(item);
    };

    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QString base = texte(row, cID) + " - " + texte(row, cDesignation);

        if (texte(row, cEtat) == "En panne")
            ajouter(base + " : équipement en panne", kNavy, true);

        if (texte(row, cUtilisation).remove('%').trimmed().toInt() >= 85)
            ajouter(base + " : niveau d'utilisation élevé (" + texte(row, cUtilisation) + ")",
                    QColor("#E5484D"), false);

        if (texte(row, cEtat) != "En panne") {
            const Predictif::Resultat r = analyserLigne(row);
            if (r.score >= 65)
                ajouter(base + QString(" : risque de panne élevé (score %1/100)").arg(r.score),
                        QColor("#E5484D"), true);
            if (r.jourPanneEstimee > 0) {
                const int j = int(r.jourPanneEstimee - today.toJulianDay());
                if (j <= 30)
                    ajouter(base + QString(" : panne probable d'ici %1 jour(s)").arg(j),
                            QColor("#E5484D"), false);
            }
        }

        const QDate prochaine = QDate::fromString(texte(row, cProchaine), kFmt);
        if (!prochaine.isValid())
            continue;
        const int jours = today.daysTo(prochaine);
        if (jours < 0)
            ajouter(base + QString(" : maintenance dépassée de %1 jour(s)").arg(-jours), kBlue, true);
        else if (jours <= 7)
            ajouter(base + QString(" : maintenance prévue dans %1 jour(s)").arg(jours), kGreen, false);
    }

    m_onglets->setTabText(1, QString("Smart Alert (%1)").arg(m_alertes->count()));
}

// Classement des équipements par risque décroissant (maintenance prédictive).
void GestionEquipement::rafraichirClassementIA()
{
    struct Ligne { int score; int niveau; QString id; QString nom; };
    QList<Ligne> liste;
    for (int row = 0; row < m_table->rowCount(); ++row) {
        const Predictif::Resultat r = analyserLigne(row);
        liste.append(Ligne{r.score, int(r.niveau), texte(row, cID), texte(row, cDesignation)});
    }
    std::sort(liste.begin(), liste.end(), [](const Ligne &a, const Ligne &b) {
        return a.score != b.score ? a.score > b.score : a.id.toLongLong() < b.id.toLongLong();
    });

    m_iaClassement->clear();
    for (int i = 0; i < liste.size() && i < 8; ++i) {
        const Ligne &l = liste.at(i);
        auto *item = new QListWidgetItem(QString("%1 - %2   ·   %3/100  %4")
                                             .arg(l.id, l.nom, QString::number(l.score), texteNiveau(l.niveau)));
        item->setData(Qt::UserRole, l.id);
        item->setForeground(QColor(couleurNiveau(l.niveau)));
        m_iaClassement->addItem(item);
    }
}

// Recherche par ID, nom, type, localisation ou état (sans tenir compte des accents ni de la casse).
// Un nombre seul est comparé exactement à l'ID : « 1 » ne ramène pas l'équipement 10.
void GestionEquipement::appliquerFiltre()
{
    const QString q = normaliser(m_recherche->text().trimmed());
    const bool tousTypes = m_filtreType->currentIndex() == 0;
    const bool tousEtats = m_filtreEtat->currentIndex() == 0;
    const int colonnes[] = {cID, cDesignation, cType, cLocalisation, cEtat};

    bool numerique = !q.isEmpty();
    for (const QChar &c : q) {
        if (!c.isDigit()) {
            numerique = false;
            break;
        }
    }

    for (int row = 0; row < m_table->rowCount(); ++row) {
        bool okTexte = q.isEmpty();
        for (int col : colonnes) {
            if (okTexte)
                break;
            const QString valeur = normaliser(texte(row, col));
            if (col == cID)
                okTexte = numerique ? (valeur == q) : valeur.contains(q);
            else
                okTexte = valeur.contains(q);
        }
        const bool okType = tousTypes || texte(row, cType) == m_filtreType->currentText();
        const bool okEtat = tousEtats || texte(row, cEtat) == m_filtreEtat->currentText();
        m_table->setRowHidden(row, !(okTexte && okType && okEtat));
    }
}

void GestionEquipement::rafraichirTout()
{
    rafraichirResume();
    rafraichirListeIA();
    rafraichirClassementIA();
    rafraichirAlertes();
    appliquerFiltre();
    onAnalyser();
}