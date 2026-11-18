#include "statistiques_dialog.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtGlobal>
#include <algorithm>

namespace {

const QColor kNavy("#0B2A55");
const QColor kBlue("#2F6FED");
const QColor kGreen("#1FA971");
const QColor kRed("#E5484D");
const QColor kGrey("#7D8CA8");

const char *kMois[12] = {"Janv", "Févr", "Mars", "Avr", "Mai", "Juin",
                         "Juil", "Août", "Sept", "Oct", "Nov", "Déc"};

// ------------------------------------------------------------------ graphiques
// Anneau : répartition opérationnel / maintenance / panne
class DonutChart : public QWidget
{
public:
    explicit DonutChart(QWidget *parent = nullptr) : QWidget(parent) { setFixedSize(150, 150); }

    void setValeurs(int op, int maint, int panne)
    {
        m_op = op; m_maint = maint; m_panne = panne;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const int total = m_op + m_maint + m_panne;
        const qreal epaisseur = 18;
        const QRectF anneau = QRectF(rect()).adjusted(epaisseur / 2 + 2, epaisseur / 2 + 2,
                                                      -epaisseur / 2 - 2, -epaisseur / 2 - 2);
        if (total == 0) {
            p.setPen(QPen(QColor("#E6EEFA"), epaisseur));
            p.drawEllipse(anneau);
        } else {
            struct Seg { int n; QColor c; };
            const Seg segs[] = {{m_op, kGreen}, {m_maint, kBlue}, {m_panne, kNavy}};
            int debut = 90 * 16;
            for (const Seg &s : segs) {
                if (s.n <= 0)
                    continue;
                const int angle = qRound(-360.0 * 16 * s.n / total);
                p.setPen(QPen(s.c, epaisseur, Qt::SolidLine, Qt::FlatCap));
                p.drawArc(anneau, debut, angle);
                debut += angle;
            }
        }
        p.setPen(kNavy);
        QFont f = font();
        f.setPixelSize(30);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(0, 44, width(), 36), Qt::AlignCenter, QString::number(total));
        f.setPixelSize(10);
        f.setBold(false);
        p.setFont(f);
        p.setPen(kGrey);
        p.drawText(QRectF(0, 80, width(), 16), Qt::AlignCenter, "ÉQUIPEMENTS");
    }

private:
    int m_op = 0, m_maint = 0, m_panne = 0;
};

// Jauge horizontale pour le taux de disponibilité
class Jauge : public QWidget
{
public:
    explicit Jauge(QWidget *parent = nullptr) : QWidget(parent) { setFixedHeight(46); }

    void setValeur(double pct)
    {
        m_pct = qBound(0.0, pct, 100.0);
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF piste(0, 26, width(), 14);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#E6EEFA"));
        p.drawRoundedRect(piste, 7, 7);

        const QColor c = m_pct >= 80 ? kGreen : (m_pct >= 50 ? kBlue : kRed);
        p.setBrush(c);
        if (m_pct > 0)
            p.drawRoundedRect(QRectF(piste.left(), piste.top(), piste.width() * m_pct / 100.0, piste.height()), 7, 7);

        QFont f = font();
        f.setPixelSize(20);
        f.setBold(true);
        p.setFont(f);
        p.setPen(c);
        p.drawText(QRectF(0, 0, width(), 24), Qt::AlignLeft | Qt::AlignVCenter,
                   QString::number(m_pct, 'f', 1) + " %");
    }

private:
    double m_pct = 0;
};

// Histogramme vertical (pannes par mois)
class BarChart : public QWidget
{
public:
    explicit BarChart(QWidget *parent = nullptr) : QWidget(parent) { setMinimumHeight(220); }

    void setDonnees(const QStringList &labels, const QList<int> &valeurs, const QString &vide)
    {
        m_labels = labels;
        m_valeurs = valeurs;
        m_vide = vide;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QFont f = font();
        f.setPixelSize(11);
        p.setFont(f);

        const int n = m_valeurs.size();
        int maxV = 0;
        for (int v : m_valeurs)
            maxV = qMax(maxV, v);
        if (n == 0 || maxV == 0) {
            p.setPen(kGrey);
            p.drawText(rect(), Qt::AlignCenter, m_vide);
            return;
        }

        const QRectF zone(8, 24, width() - 16, height() - 24 - 26);
        const qreal case_ = zone.width() / n;
        const qreal largeur = case_ * 0.55;

        p.setPen(QPen(QColor("#E3EBF7"), 1));
        p.drawLine(zone.bottomLeft(), zone.bottomRight());

        for (int i = 0; i < n; ++i) {
            const int v = m_valeurs.at(i);
            const qreal h = zone.height() * v / maxV;
            const QRectF barre(zone.left() + case_ * i + (case_ - largeur) / 2,
                               zone.bottom() - h, largeur, h);
            if (v > 0) {
                p.setPen(Qt::NoPen);
                p.setBrush(v == maxV ? kNavy : kBlue);
                p.drawRoundedRect(barre, 4, 4);
                p.setPen(kNavy);
                p.drawText(QRectF(barre.left() - 10, barre.top() - 18, largeur + 20, 16),
                           Qt::AlignCenter, QString::number(v));
            }
            p.setPen(kGrey);
            p.drawText(QRectF(zone.left() + case_ * i, zone.bottom() + 5, case_, 16),
                       Qt::AlignCenter, i < m_labels.size() ? m_labels.at(i) : QString());
        }
    }

private:
    QStringList m_labels;
    QList<int> m_valeurs;
    QString m_vide;
};

// Barres horizontales (classements)
class HBarChart : public QWidget
{
public:
    explicit HBarChart(QWidget *parent = nullptr) : QWidget(parent) { setMinimumHeight(60); }

    void setDonnees(const QList<QPair<QString, int>> &lignes, const QColor &couleur, const QString &vide)
    {
        m_lignes = lignes;
        m_couleur = couleur;
        m_vide = vide;
        setMinimumHeight(qMax(60, int(lignes.size()) * 30 + 8));
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QFont f = font();
        f.setPixelSize(12);
        p.setFont(f);

        if (m_lignes.isEmpty()) {
            p.setPen(kGrey);
            p.drawText(rect(), Qt::AlignCenter, m_vide);
            return;
        }

        int maxV = 1;
        for (const auto &l : m_lignes)
            maxV = qMax(maxV, l.second);

        const int nomLarg = qMax(90, width() * 38 / 100);
        const int valLarg = 34;
        const QFontMetrics fm(f);

        for (int i = 0; i < m_lignes.size(); ++i) {
            const qreal y = 4 + i * 30;
            p.setPen(kNavy);
            p.drawText(QRectF(0, y, nomLarg - 8, 22), Qt::AlignLeft | Qt::AlignVCenter,
                       fm.elidedText(m_lignes.at(i).first, Qt::ElideRight, nomLarg - 8));

            const qreal zoneBarre = width() - nomLarg - valLarg;
            const qreal l = zoneBarre * m_lignes.at(i).second / maxV;
            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#E6EEFA"));
            p.drawRoundedRect(QRectF(nomLarg, y + 5, zoneBarre, 12), 6, 6);
            p.setBrush(m_couleur);
            if (l > 0)
                p.drawRoundedRect(QRectF(nomLarg, y + 5, qMax<qreal>(l, 12), 12), 6, 6);

            p.setPen(kNavy);
            p.drawText(QRectF(width() - valLarg + 6, y, valLarg - 6, 22),
                       Qt::AlignLeft | Qt::AlignVCenter, QString::number(m_lignes.at(i).second));
        }
    }

private:
    QList<QPair<QString, int>> m_lignes;
    QColor m_couleur = kBlue;
    QString m_vide;
};

// ---------------------------------------------------------------------- aides UI
QFrame *carte(const QString &titre, QWidget *contenu)
{
    auto *c = new QFrame;
    c->setObjectName("card");
    auto *l = new QVBoxLayout(c);
    l->setContentsMargins(16, 12, 16, 14);
    l->setSpacing(8);
    auto *t = new QLabel(titre);
    t->setObjectName("section");
    l->addWidget(t);
    l->addWidget(contenu, 1);
    return c;
}

QFrame *carteKpi(const QString &titre, const QString &valeur, const QString &couleur)
{
    auto *c = new QFrame;
    c->setObjectName("card");
    auto *l = new QVBoxLayout(c);
    l->setContentsMargins(14, 10, 14, 10);
    l->setSpacing(2);
    auto *t = new QLabel(titre);
    t->setObjectName("kpiTitre");
    auto *v = new QLabel(valeur);
    v->setObjectName("kpiValeur");
    v->setStyleSheet("color:" + couleur + ";");
    l->addWidget(t);
    l->addWidget(v);
    return c;
}

const char *kStyleStats = R"CSS(
    QDialog { background:#F6F9FF; }
    QWidget { color:#0B2A55; font-family:'Segoe UI'; font-size:12px; }
    QScrollArea { border:none; background:#F6F9FF; }
    QWidget#contenu { background:#F6F9FF; }
    QLabel { background:transparent; }
    QFrame#card { background:white; border:1px solid #E3EBF7; border-radius:14px; }
    QLabel#titre { font-size:20px; font-weight:800; }
    QLabel#sousTitre { color:#6C7C98; }
    QLabel#section { font-size:14px; font-weight:800; }
    QLabel#kpiTitre { font-size:11px; color:#7D8CA8; }
    QLabel#kpiValeur { font-size:26px; font-weight:800; }
    QLabel#note { color:#B26A00; background:#FFF4E0; border-radius:8px; padding:8px 10px; }
    QPushButton { background:#2F6FED; color:white; border:none; border-radius:9px;
        padding:0 22px; min-height:34px; font-weight:800; }
    QPushButton:hover { background:#2461D6; }
)CSS";

} // namespace

// ------------------------------------------------------------------ calcul
StatsEquipements calculerStats(const QList<Equipement> &equipements,
                               const QList<Evenement> &evenements,
                               bool historiqueDisponible,
                               const QString &messageHistorique,
                               const QDate &aujourdhui)
{
    StatsEquipements s;
    s.total = equipements.size();

    QMap<QString, int> parType;
    QMap<qlonglong, QString> noms;
    for (const Equipement &e : equipements) {
        if (e.etat == "Opérationnel")
            ++s.operationnels;
        else if (e.etat == "En panne")
            ++s.enPanne;
        else
            ++s.enMaintenance;
        parType[e.type.isEmpty() ? QString("Non défini") : e.type]++;
        noms[e.id] = e.designation;
    }
    s.disponibilite = s.total > 0 ? 100.0 * s.operationnels / s.total : 0.0;

    for (auto it = parType.constBegin(); it != parType.constEnd(); ++it)
        s.parType.append({it.key(), it.value()});
    std::sort(s.parType.begin(), s.parType.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.second != b.second ? a.second > b.second : a.first < b.first;
              });

    // 12 mois glissants, du plus ancien au plus récent
    const int moisCourant = aujourdhui.year() * 12 + (aujourdhui.month() - 1);
    for (int i = 11; i >= 0; --i) {
        const int m = moisCourant - i;
        s.moisLabels << QString("%1 %2").arg(QString::fromUtf8(kMois[m % 12])).arg((m / 12) % 100, 2, 10, QChar('0'));
        s.pannesParMois << 0;
    }

    s.historiqueDisponible = historiqueDisponible;
    s.messageHistorique = messageHistorique;

    QMap<qlonglong, int> parEquipement;
    if (historiqueDisponible) {
        for (const Evenement &ev : evenements) {
            if (ev.type != "Panne" || !ev.date.isValid())
                continue;
            const int m = ev.date.year() * 12 + (ev.date.month() - 1);
            const int idx = 11 - (moisCourant - m);
            if (idx < 0 || idx > 11)
                continue;                               // hors des 12 derniers mois
            s.pannesParMois[idx]++;
            ++s.totalPannes12Mois;
            parEquipement[ev.idEquipement]++;
        }
        s.moyennePannesParMois = s.totalPannes12Mois / 12.0;

        QList<QPair<QString, int>> classement;
        for (auto it = parEquipement.constBegin(); it != parEquipement.constEnd(); ++it) {
            const QString nom = noms.contains(it.key())
                                    ? QString("%1 - %2").arg(it.key()).arg(noms.value(it.key()))
                                    : QString("Équipement %1 (supprimé)").arg(it.key());
            classement.append({nom, it.value()});
        }
        std::sort(classement.begin(), classement.end(),
                  [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                      return a.second != b.second ? a.second > b.second : a.first < b.first;
                  });
        s.pannesParEquipement = classement.mid(0, 5);
    }
    return s;
}

// ------------------------------------------------------------------ fenêtre
StatistiquesDialog::StatistiquesDialog(const StatsEquipements &s, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Statistiques des équipements");
    resize(1020, 720);
    setStyleSheet(kStyleStats);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 12);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll, 1);

    auto *contenu = new QWidget;
    contenu->setObjectName("contenu");
    scroll->setWidget(contenu);
    auto *root = new QVBoxLayout(contenu);
    root->setContentsMargins(20, 16, 20, 8);
    root->setSpacing(12);

    auto *titre = new QLabel("STATISTIQUES DES ÉQUIPEMENTS");
    titre->setObjectName("titre");
    auto *sous = new QLabel("État du parc, taux de disponibilité et fréquence des pannes");
    sous->setObjectName("sousTitre");
    root->addWidget(titre);
    root->addWidget(sous);

    // --- indicateurs
    auto *kpi = new QHBoxLayout;
    kpi->setSpacing(12);
    kpi->addWidget(carteKpi("Total équipements", QString::number(s.total), "#0B2A55"));
    kpi->addWidget(carteKpi("Opérationnels", QString::number(s.operationnels), "#1FA971"));
    kpi->addWidget(carteKpi("En maintenance", QString::number(s.enMaintenance), "#2F6FED"));
    kpi->addWidget(carteKpi("En panne", QString::number(s.enPanne), "#E5484D"));
    kpi->addWidget(carteKpi("Taux de disponibilité", QString::number(s.disponibilite, 'f', 1) + " %",
                            s.disponibilite >= 80 ? "#1FA971" : (s.disponibilite >= 50 ? "#2F6FED" : "#E5484D")));
    root->addLayout(kpi);

    // --- ligne 2 : états + disponibilité / types
    auto *etatWidget = new QWidget;
    auto *etatLayout = new QHBoxLayout(etatWidget);
    etatLayout->setContentsMargins(0, 0, 0, 0);
    etatLayout->setSpacing(20);
    auto *donut = new DonutChart;
    donut->setValeurs(s.operationnels, s.enMaintenance, s.enPanne);
    etatLayout->addWidget(donut, 0, Qt::AlignTop);
    auto *legende = new QVBoxLayout;
    legende->setSpacing(10);
    auto ligneLegende = [](const QString &titre, int n, const QString &couleur) {
        auto *l = new QLabel(QString("<span style='color:%1;font-weight:700;'>●&nbsp;&nbsp;%2 : %3</span>")
                                 .arg(couleur, titre).arg(n));
        l->setTextFormat(Qt::RichText);
        return l;
    };
    legende->addWidget(ligneLegende("Opérationnels", s.operationnels, "#1FA971"));
    legende->addWidget(ligneLegende("En maintenance", s.enMaintenance, "#2F6FED"));
    legende->addWidget(ligneLegende("En panne", s.enPanne, "#0B2A55"));
    legende->addStretch();
    etatLayout->addLayout(legende, 1);

    auto *dispoWidget = new QWidget;
    auto *dispoLayout = new QVBoxLayout(dispoWidget);
    dispoLayout->setContentsMargins(0, 0, 0, 0);
    auto *jauge = new Jauge;
    jauge->setValeur(s.disponibilite);
    dispoLayout->addWidget(jauge);
    auto *def = new QLabel("Équipements opérationnels / total des équipements.");
    def->setObjectName("sousTitre");
    def->setWordWrap(true);
    dispoLayout->addWidget(def);
    auto *typesTitre = new QLabel("Répartition par type");
    typesTitre->setObjectName("section");
    dispoLayout->addSpacing(6);
    dispoLayout->addWidget(typesTitre);
    auto *types = new HBarChart;
    types->setDonnees(s.parType.mid(0, 6), kBlue, "Aucun équipement");
    dispoLayout->addWidget(types);
    dispoLayout->addStretch();

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(12);
    row2->addWidget(carte("ÉTAT DES ÉQUIPEMENTS", etatWidget), 1);
    row2->addWidget(carte("TAUX DE DISPONIBILITÉ", dispoWidget), 1);
    root->addLayout(row2);

    // --- ligne 3 : fréquence des pannes
    if (!s.historiqueDisponible) {
        auto *note = new QLabel("Historique des pannes indisponible : "
                                + (s.messageHistorique.isEmpty()
                                       ? QString("exécutez le script creation_historique.sql dans SQL Developer.")
                                       : s.messageHistorique));
        note->setObjectName("note");
        note->setWordWrap(true);
        root->addWidget(note);
    }

    auto *mois = new BarChart;
    mois->setDonnees(s.moisLabels, s.pannesParMois, "Aucune panne enregistrée sur les 12 derniers mois");
    auto *moisWidget = new QWidget;
    auto *moisLayout = new QVBoxLayout(moisWidget);
    moisLayout->setContentsMargins(0, 0, 0, 0);
    auto *resume = new QLabel(QString("%1 panne(s) sur 12 mois  ·  moyenne %2 par mois")
                                  .arg(s.totalPannes12Mois)
                                  .arg(s.moyennePannesParMois, 0, 'f', 1));
    resume->setObjectName("sousTitre");
    moisLayout->addWidget(resume);
    moisLayout->addWidget(mois, 1);

    auto *parEquip = new HBarChart;
    parEquip->setDonnees(s.pannesParEquipement, kNavy, "Aucune panne enregistrée");
    auto *equipWidget = new QWidget;
    auto *equipLayout = new QVBoxLayout(equipWidget);
    equipLayout->setContentsMargins(0, 0, 0, 0);
    equipLayout->addWidget(parEquip);
    equipLayout->addStretch();

    auto *row3 = new QHBoxLayout;
    row3->setSpacing(12);
    row3->addWidget(carte("FRÉQUENCE DES PANNES (12 DERNIERS MOIS)", moisWidget), 3);
    row3->addWidget(carte("ÉQUIPEMENTS LES PLUS TOUCHÉS", equipWidget), 2);
    root->addLayout(row3);
    root->addStretch();

    auto *fermer = new QPushButton("Fermer");
    fermer->setCursor(Qt::PointingHandCursor);
    connect(fermer, &QPushButton::clicked, this, &QDialog::accept);
    auto *bas = new QHBoxLayout;
    bas->setContentsMargins(20, 0, 20, 0);
    bas->addStretch();
    bas->addWidget(fermer);
    outer->addLayout(bas);
}
