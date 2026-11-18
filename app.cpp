#include "app.h"
#include "logo-data.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QStackedWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QIntValidator>
#include <QRegularExpression>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QIcon>
#include <QLinearGradient>
#include <QProgressBar>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include <QLocale>
#include <QPair>
#include <algorithm>
#include <cmath>

#define QS(x) QStringLiteral(x)

// ============================================================
// UTILITAIRES
// ============================================================

namespace {

QString spaced(double value)
{
    const qint64 n = qRound64(value);
    QString s = QString::number(qAbs(n));

    for (int i = s.size() - 3; i > 0; i -= 3)
        s.insert(i, QLatin1Char(' '));

    return n < 0 ? QS("-") + s : s;
}

QString fmtDate(const QDate &d)
{
    return d.isValid() ? d.toString(QS("dd/MM/yyyy")) : QS("-");
}

QString code(char prefix, int id)
{
    return QString(QLatin1Char(prefix)) + QString::number(id).rightJustified(3, QLatin1Char('0'));
}

QPixmap loadLogo()
{
    QPixmap pix;
    pix.loadFromData(kLogoPng, sizeof(kLogoPng), "PNG");
    return pix;
}

bool validEmail(const QString &email)
{
    static const QRegularExpression re(
        QS("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$"));
    return re.match(email).hasMatch();
}

QString csvq(const QString &s)
{
    QString v = s;
    v.replace(QLatin1Char('"'), QS("\"\""));
    return QS("\"") + v + QS("\"");
}

QPushButton *makeButton(const QString &text, const char *name)
{
    QPushButton *b = new QPushButton(text);
    b->setObjectName(QLatin1String(name));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QFrame *makePanel()
{
    QFrame *f = new QFrame;
    f->setObjectName("panel");
    return f;
}

QLabel *makeTitle(const QString &text)
{
    QLabel *l = new QLabel(text);
    l->setObjectName("panelTitle");
    return l;
}

QWidget *makeHeader(const QString &title, const QString &subtitle)
{
    QWidget *w = new QWidget;
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 4);
    l->setSpacing(0);

    QLabel *t = new QLabel(title);
    t->setObjectName("pageTitle");
    QLabel *s = new QLabel(subtitle);
    s->setObjectName("pageSubtitle");

    l->addWidget(t);
    l->addWidget(s);
    return w;
}

QFrame *makeCard(const QString &title, const QString &icon, const QString &iconBg,
                 QLabel **valueOut)
{
    QFrame *card = makePanel();
    card->setMinimumHeight(90);

    QHBoxLayout *layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(14);

    QLabel *iconLabel = new QLabel(icon);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(58, 58);
    iconLabel->setStyleSheet(
        QS("background:%1; border:none; border-radius:12px; font-size:26px;").arg(iconBg));

    QVBoxLayout *texts = new QVBoxLayout;
    texts->setSpacing(2);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet(QS("font-size:13px; color:#526173; border:none;"));

    QLabel *value = new QLabel(QS("0"));
    value->setStyleSheet(QS("font-size:24px; font-weight:bold; border:none;"));

    texts->addWidget(titleLabel);
    texts->addWidget(value);

    layout->addWidget(iconLabel);
    layout->addLayout(texts, 1);

    *valueOut = value;
    return card;
}

void addField(QGridLayout *grid, int row, int col, const QString &text, QWidget *field)
{
    QLabel *label = new QLabel(text);
    label->setStyleSheet(QS("font-size:12px; color:#526173; margin-top:4px;"));
    grid->addWidget(label, row * 2, col);
    grid->addWidget(field, row * 2 + 1, col);
}

QGridLayout *makeGrid(int columns)
{
    QGridLayout *g = new QGridLayout;
    g->setHorizontalSpacing(12);
    g->setVerticalSpacing(3);
    for (int i = 0; i < columns; ++i)
        g->setColumnStretch(i, 1);
    return g;
}

QDateEdit *makeDate(const QDate &d)
{
    QDateEdit *e = new QDateEdit(d);
    e->setCalendarPopup(true);
    e->setDisplayFormat(QS("dd/MM/yyyy"));
    return e;
}

QTableWidget *makeTable(const QStringList &headers)
{
    QTableWidget *t = new QTableWidget;
    t->setColumnCount(headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    t->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    t->verticalHeader()->setVisible(false);
    t->verticalHeader()->setDefaultSectionSize(40);
    t->setShowGrid(false);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    return t;
}

// pastille coloree dans une cellule de tableau
void setPill(QTableWidget *t, int row, int col, const QString &text,
             const QString &bg, const QString &fg)
{
    QWidget *wrap = new QWidget;
    wrap->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    QHBoxLayout *l = new QHBoxLayout(wrap);
    l->setContentsMargins(6, 6, 6, 6);

    QLabel *pill = new QLabel(text);
    pill->setAlignment(Qt::AlignCenter);
    pill->setStyleSheet(
        QS("background:%1; color:%2; border:none; border-radius:5px;"
           "font-weight:bold; font-size:11px;").arg(bg, fg));

    l->addWidget(pill);
    t->setCellWidget(row, col, wrap);
}

// id de la ligne selectionnee (stocke dans Qt::UserRole de la colonne 0)
int rowId(const QTableWidget *t)
{
    const QList<QTableWidgetItem *> sel = t->selectedItems();
    if (sel.isEmpty())
        return -1;

    const QTableWidgetItem *item = t->item(sel.first()->row(), 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void selectById(QTableWidget *t, int id)
{
    for (int row = 0; row < t->rowCount(); ++row)
    {
        const QTableWidgetItem *item = t->item(row, 0);
        if (item && item->data(Qt::UserRole).toInt() == id)
        {
            t->selectRow(row);
            return;
        }
    }
}

QTableWidgetItem *idItem(const QString &text, int id)
{
    QTableWidgetItem *it = new QTableWidgetItem(text);
    it->setData(Qt::UserRole, id);
    return it;
}

void boldRed(QTableWidgetItem *it)
{
    it->setForeground(QColor(0xB42318));
    QFont f = it->font();
    f.setBold(true);
    it->setFont(f);
}

void fillIdCombo(QComboBox *combo, const QVector<QPair<QString, int>> &items)
{
    const QVariant previous = combo->currentData();

    combo->blockSignals(true);
    combo->clear();

    for (const auto &it : items)
        combo->addItem(it.first, it.second);

    int index = previous.isValid() ? combo->findData(previous) : -1;
    if (index < 0 && combo->count() > 0)
        index = 0;

    combo->setCurrentIndex(index);
    combo->blockSignals(false);
}

void exportTo(QWidget *parent, StatusBox *status, const QString &what,
              const QString &fileName, const QString &header, const QStringList &lines)
{
    const QString path = QFileDialog::getSaveFileName(
        parent, QS("Exporter"), fileName, QS("Fichiers Excel / CSV (*.csv)"));

    if (path.isEmpty())
        return;

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
    {
        status->say(QS("Impossible de cr\u00e9er le fichier."), StatusBox::Error);
        return;
    }

    f.write("\xEF\xBB\xBF"); // BOM : accents corrects dans Excel
    f.write((header + QS("\n") + lines.join(QS("\n")) + QS("\n")).toUtf8());
    f.close();

    status->say(QS("La liste (%1) a \u00e9t\u00e9 export\u00e9e.").arg(what), StatusBox::Success);
}

QColor alertColor(int level)
{
    if (level == 0) return QColor(0xB42318);
    if (level == 1) return QColor(0xC45B00);
    return QColor(0x7A1FA2);
}

// style du panneau de risque (barre + pastille de niveau)
void applyRisk(QProgressBar *bar, QLabel *levelLabel, int score)
{
    QString bg, fg, col;

    if (score >= 70)
    {
        bg = QS("#FDE8E8"); fg = QS("#B42318"); col = QS("#EF4444");
    }
    else if (score >= 40)
    {
        bg = QS("#FFE9D2"); fg = QS("#C45B00"); col = QS("#FF8A34");
    }
    else
    {
        bg = QS("#D9FBEA"); fg = QS("#138A5B"); col = QS("#20B486");
    }

    bar->setValue(score);
    bar->setStyleSheet(
        QS("QProgressBar { background:#E5EAF0; border:none; border-radius:5px; }"
           "QProgressBar::chunk { background:%1; border-radius:5px; }").arg(col));

    levelLabel->setStyleSheet(
        QS("background:%1; color:%2; border-radius:6px; padding:8px 14px;"
           "font-size:15px; font-weight:bold;").arg(bg, fg));
}

QString etatOp()    { return QS("Op\u00e9rationnel"); }
QString etatPanne() { return QS("En panne"); }
QString etatMaint() { return QS("En maintenance"); }

QString catGrossiste()  { return QS("Grossiste"); }
QString catDetaillant() { return QS("D\u00e9taillant"); }
QString catEntreprise() { return QS("Entreprise"); }

QString tMin() { return QS("Eau min\u00e9rale"); }
QString tGaz() { return QS("Eau gazeuse"); }
QString tAro() { return QS("Eau aromatis\u00e9e"); }

QString sPresent() { return QS("Pr\u00e9sent"); }
QString sAbsent()  { return QS("Absent"); }
QString sConge()   { return QS("En cong\u00e9"); }

} // namespace

// ============================================================
// HISTOGRAMME
// ============================================================

BarChart::BarChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(140);
    setMinimumWidth(260);
    m_colors = {QColor("#20B486"), QColor("#FF8A34"), QColor("#2678E8")};
}

void BarChart::setTitle(const QString &title) { m_title = title; update(); }
void BarChart::setColors(const QVector<QColor> &colors) { m_colors = colors; update(); }

void BarChart::setData(const QVector<double> &values, const QVector<QString> &labels)
{
    m_values = values;
    m_labels = labels;
    update();
}

void BarChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QColor("#172B4D"));
    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.drawText(QRect(14, 8, width() - 28, 24),
               Qt::AlignLeft | Qt::AlignVCenter, m_title);

    if (m_values.isEmpty() || m_colors.isEmpty())
        return;

    double maxV = 0;
    for (double v : m_values)
        maxV = qMax(maxV, v);
    if (maxV <= 0)
        maxV = 1;

    // echelle "propre" sur 4 intervalles
    const double raw = maxV / 4.0;
    const double mag = std::pow(10.0, std::floor(std::log10(raw)));
    const double norm = raw / mag;

    double nice = 10;
    if (norm <= 1)      nice = 1;
    else if (norm <= 2) nice = 2;
    else if (norm <= 5) nice = 5;

    double step = nice * mag;
    if (step < 1)
        step = 1;
    const double axisMax = step * 4.0;

    p.setFont(QFont("Arial", 8));

    const int left = qMax(36, p.fontMetrics().horizontalAdvance(spaced(axisMax)) + 18);
    const int right = 15;
    const int top = 48;
    const int bottom = 30;

    const int gw = width() - left - right;
    const int gh = height() - top - bottom;
    if (gw <= 0 || gh <= 0)
        return;

    for (int i = 0; i <= 4; ++i)
    {
        const int y = top + gh - gh * i / 4;

        p.setPen(QColor("#E5EAF0"));
        p.drawLine(left, y, width() - right, y);

        p.setPen(QColor("#526173"));
        p.drawText(QRect(0, y - 9, left - 8, 18),
                   Qt::AlignRight | Qt::AlignVCenter, spaced(step * i));
    }

    const int count = m_values.size();
    const int spacing = gw / count;
    const int bw = qMax(18, spacing / 2);

    for (int i = 0; i < count; ++i)
    {
        const int bh = qMax(3, static_cast<int>((m_values[i] / axisMax) * gh));
        const int x = left + i * spacing + (spacing - bw) / 2;
        const int y = top + gh - bh;

        p.setBrush(m_colors[i % m_colors.size()]);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRect(x, y, bw, bh), 4, 4);

        p.setPen(QColor("#172B4D"));
        p.setFont(QFont("Arial", 8, QFont::Bold));
        p.drawText(QRect(x - 25, y - 20, bw + 50, 18), Qt::AlignCenter, spaced(m_values[i]));

        p.setFont(QFont("Arial", 8));
        p.setPen(QColor("#526173"));
        p.drawText(QRect(x - 30, top + gh + 6, bw + 60, 20), Qt::AlignCenter,
                   i < m_labels.size() ? m_labels[i] : QString());
    }
}

PieChart::PieChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(120);
    m_colors = {QColor("#20B486"), QColor("#FF8A34"), QColor("#2678E8")};
}

void PieChart::setTitle(const QString &title) { m_title = title; update(); }
void PieChart::setColors(const QVector<QColor> &colors) { m_colors = colors; update(); }

void PieChart::setData(const QVector<double> &values, const QVector<QString> &labels)
{
    m_values = values;
    m_labels = labels;
    update();
}

void PieChart::paintEvent(QPaintEvent *)
{
    const double kPi = 3.14159265358979323846;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QColor("#172B4D"));
    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.drawText(QRect(14, 8, width() - 28, 24),
               Qt::AlignLeft | Qt::AlignVCenter, m_title);

    double total = 0;
    for (double v : m_values)
        total += v;

    if (total <= 0 || m_colors.isEmpty())
    {
        p.setFont(QFont("Arial", 9));
        p.drawText(rect(), Qt::AlignCenter, QS("Aucune donn\u00e9e"));
        return;
    }

    const int topArea = 38;
    const int d = qMax(30, qMin(height() - topArea - 10, width() / 2 - 20));
    const QRect pieRect(22, topArea + (height() - topArea - d) / 2, d, d);

    double startDeg = 90.0;

    for (int i = 0; i < m_values.size(); ++i)
    {
        if (m_values[i] <= 0)
            continue;

        const double sweep = m_values[i] / total * 360.0;

        p.setBrush(m_colors[i % m_colors.size()]);
        p.setPen(QPen(Qt::white, 2));
        p.drawPie(pieRect, static_cast<int>(startDeg * 16), static_cast<int>(-sweep * 16));

        if (sweep >= 18.0)
        {
            const double mid = (startDeg - sweep / 2.0) * kPi / 180.0;
            const double cx = pieRect.center().x() + d * 0.30 * std::cos(mid);
            const double cy = pieRect.center().y() - d * 0.30 * std::sin(mid);

            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 8, QFont::Bold));
            p.drawText(QRect(static_cast<int>(cx) - 22, static_cast<int>(cy) - 10, 44, 20),
                       Qt::AlignCenter,
                       QS("%1%").arg(qRound(m_values[i] / total * 100.0)));
        }

        startDeg -= sweep;
    }

    const int legendX = pieRect.right() + 30;
    const int legendTop = pieRect.center().y() - (m_values.size() * 26) / 2;

    p.setFont(QFont("Arial", 9));

    for (int i = 0; i < m_values.size(); ++i)
    {
        const int y = legendTop + i * 26;

        p.setBrush(m_colors[i % m_colors.size()]);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QRect(legendX, y + 5, 12, 12));

        p.setPen(QColor("#172B4D"));
        p.drawText(QRect(legendX + 20, y, width() - legendX - 24, 22),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QS("%1 : %2 (%3%)")
                       .arg(i < m_labels.size() ? m_labels[i] : QString())
                       .arg(static_cast<int>(m_values[i]))
                       .arg(qRound(m_values[i] / total * 100.0)));
    }
}


BottleWidget::BottleWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(190);
}

void BottleWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const double h = height() - 16;
    const double bw = qMin(width() * 0.55, h * 0.62);
    const double cx = width() / 2.0;
    const double top = 8;

    const double capH = h * 0.08;
    const double neckH = h * 0.10;
    const double neckW = bw * 0.55;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#2F8DE4"));
    p.drawRoundedRect(QRectF(cx - neckW / 2, top, neckW, capH), 3, 3);

    p.setBrush(QColor(150, 205, 255, 190));
    p.drawRect(QRectF(cx - neckW * 0.45, top + capH, neckW * 0.9, neckH));

    const QRectF body(cx - bw / 2, top + capH + neckH, bw, h - capH - neckH);

    QLinearGradient grad(body.topLeft(), body.bottomRight());
    grad.setColorAt(0.0, QColor(150, 210, 255, 230));
    grad.setColorAt(1.0, QColor(35, 120, 215, 240));

    p.setBrush(grad);
    p.drawRoundedRect(body, bw * 0.25, bw * 0.25);

    p.setBrush(QColor(255, 255, 255, 80));
    p.drawRoundedRect(QRectF(body.left() + bw * 0.12, body.top() + body.height() * 0.1,
                             bw * 0.12, body.height() * 0.6), 5, 5);

    p.setBrush(QColor(255, 255, 255, 110));
    p.drawEllipse(QPointF(body.left() + bw * 0.6, body.bottom() - 22), 4, 4);
    p.drawEllipse(QPointF(body.left() + bw * 0.75, body.bottom() - 40), 3, 3);
    p.drawEllipse(QPointF(body.left() + bw * 0.45, body.bottom() - 55), 2.5, 2.5);
}


StockCircle::StockCircle(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(145, 145);
    setMaximumSize(175, 175);
}

void StockCircle::setValues(int faible, int moyen, int bon)
{
    m_faible = faible;
    m_moyen = moyen;
    m_bon = bon;
    update();
}

void StockCircle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int total = m_faible + m_moyen + m_bon;
    const int size = qMin(width(), height()) - 16;
    const QRectF rect((width() - size) / 2.0, (height() - size) / 2.0, size, size);

    p.setPen(QPen(QColor("#E7EDF4"), 14));
    p.drawEllipse(rect);

    if (total > 0)
    {
        const double a1 = 360.0 * m_faible / total;
        const double a2 = 360.0 * m_moyen / total;
        const double a3 = 360.0 - a1 - a2;

        p.setPen(QPen(QColor("#EF4444"), 14));
        p.drawArc(rect, 90 * 16, -qRound(a1 * 16));

        p.setPen(QPen(QColor("#F7C948"), 14));
        p.drawArc(rect, qRound((90 - a1) * 16), -qRound(a2 * 16));

        p.setPen(QPen(QColor("#20B486"), 14));
        p.drawArc(rect, qRound((90 - a1 - a2) * 16), -qRound(a3 * 16));
    }

    const QRectF center(rect.left() + 24, rect.top() + 24,
                        rect.width() - 48, rect.height() - 48);

    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawEllipse(center);

    p.setPen(QColor("#172B4D"));
    QFont font = p.font();
    font.setBold(true);
    font.setPointSize(21);
    p.setFont(font);
    p.drawText(center, Qt::AlignCenter, QString::number(total));

    font.setBold(false);
    font.setPointSize(8);
    p.setFont(font);
    p.drawText(QRectF(center.left(), center.center().y() + 25, center.width(), 18),
               Qt::AlignCenter, QS("PRODUITS"));
}

// ============================================================
// BARRE DE MESSAGES (+ / -)
// ============================================================

StatusBox::StatusBox(const QString &hint, QWidget *parent)
    : QFrame(parent), m_hint(hint)
{
    setObjectName("statusBox");
    setMinimumHeight(52);

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(12, 6, 8, 6);
    lay->setSpacing(8);

    m_label = new QLabel;
    m_label->setWordWrap(true);

    m_no = new QPushButton(QString(QChar(0x2212)));
    m_no->setObjectName("cancelBtn");
    m_no->setToolTip(QS("Annuler (garder comme avant)"));
    m_no->setCursor(Qt::PointingHandCursor);
    m_no->setFixedSize(42, 34);

    m_ok = new QPushButton(QS("+"));
    m_ok->setObjectName("confirmBtn");
    m_ok->setToolTip(QS("Confirmer"));
    m_ok->setCursor(Qt::PointingHandCursor);
    m_ok->setFixedSize(42, 34);

    lay->addWidget(m_label, 1);
    lay->addWidget(m_no);
    lay->addWidget(m_ok);

    connect(m_ok, &QPushButton::clicked, this, [this]()
            {
                std::function<void()> a = m_action;
                m_action = nullptr;
                if (a)
                    a();
            });
    connect(m_no, &QPushButton::clicked, this, [this]() { cancel(false); });

    reset();
}

void StatusBox::say(const QString &text, Kind kind)
{
    QString bg, fg;

    switch (kind)
    {
    case Success: bg = QS("#D9FBEA"); fg = QS("#138A5B"); break;
    case Error:   bg = QS("#FDE8E8"); fg = QS("#B42318"); break;
    case Confirm: bg = QS("#FFF4D6"); fg = QS("#7A5200"); break;
    default:      bg = QS("#EEF3F8"); fg = QS("#526173"); break;
    }

    setStyleSheet(QS("QFrame#statusBox { background:%1; border:none; border-radius:8px; }").arg(bg));

    m_label->setStyleSheet(
        QS("color:%1; font-size:12px; font-weight:%2; background:transparent; border:none;")
            .arg(fg, kind == Info ? QS("normal") : QS("bold")));

    m_label->setText(text);

    m_ok->setVisible(kind == Confirm);
    m_no->setVisible(kind == Confirm);
}

void StatusBox::ask(const QString &text, std::function<void()> action)
{
    m_action = action;
    say(text, Confirm);
}

void StatusBox::cancel(bool silent)
{
    if (!m_action)
        return;

    m_action = nullptr;

    if (silent)
        reset();
    else
        say(QS("Annul\u00e9 : rien n'a \u00e9t\u00e9 modifi\u00e9."), Info);
}

void StatusBox::reset()
{
    m_action = nullptr;
    say(m_hint, Info);
}

// ============================================================
// MODULE PRODUITS
// ============================================================

ProduitPage::ProduitPage(QWidget *parent)
    : Page(parent)
{
    load();
    build();
    refresh();
    clearForm();
}

QString ProduitPage::summary() const
{
    int total = 0;
    for (const Produit &p : produits)
        total += p.stock;

    return QS("%1 produits \u2022 %2 unit\u00e9s en stock").arg(produits.size()).arg(spaced(total));
}

void ProduitPage::load()
{
    const QDate t = QDate::currentDate();

    produits.clear();
    produits.append(Produit{1, QS("Eau Cristal 0.5L"), tMin(), 0.800, 4200, t.addDays(-20), t.addDays(345)});
    produits.append(Produit{2, QS("Eau Cristal 1.5L"), tMin(), 1.200, 2800, t.addDays(-19), t.addDays(346)});
    produits.append(Produit{3, QS("Eau Gazeuse 1L"),   tGaz(), 1.500, 1500, t.addDays(-18), t.addDays(347)});
    produits.append(Produit{4, QS("Eau Fra\u00eeche"), tAro(), 1.800,  700, t.addDays(-17), t.addDays(348)});
}

const Produit *ProduitPage::find(int id) const
{
    for (const Produit &p : produits)
        if (p.id == id)
            return &p;
    return nullptr;
}

void ProduitPage::build()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 18, 22, 18);
    root->setSpacing(12);

    // ---------- En-tete + horloge ----------
    QHBoxLayout *header = new QHBoxLayout;
    header->addWidget(makeHeader(QS("\U0001F4E6  Gestion des Produits"),
                                 QS("Gestion du stock et suivi de la production")));
    header->addStretch();

    QVBoxLayout *dl = new QVBoxLayout;
    lblDate = new QLabel;
    lblHeure = new QLabel;
    lblDate->setAlignment(Qt::AlignRight);
    lblHeure->setAlignment(Qt::AlignRight);
    lblDate->setStyleSheet(QS("color:#526173; font-size:12px;"));
    lblHeure->setStyleSheet(QS("font-size:15px; font-weight:bold;"));
    dl->addWidget(lblDate);
    dl->addWidget(lblHeure);
    header->addLayout(dl);
    root->addLayout(header);

    QTimer *clock = new QTimer(this);
    connect(clock, &QTimer::timeout, this, [this]()
            {
                const QDateTime now = QDateTime::currentDateTime();
                lblDate->setText(QLocale(QLocale::French).toString(now.date(), QS("dddd dd MMMM yyyy")));
                lblHeure->setText(now.time().toString(QS("HH:mm:ss")));
            });
    clock->setInterval(1000);
    clock->start();

    // Affichage immédiat de la date et de l'heure
    const QDateTime now = QDateTime::currentDateTime();
    lblDate->setText(
        QLocale(QLocale::French).toString(now.date(), QS("dddd dd MMMM yyyy"))
        );
    lblHeure->setText(now.time().toString(QS("HH:mm:ss")));

    // ---------- Cartes du haut ----------
    QHBoxLayout *cards = new QHBoxLayout;
    cards->setSpacing(12);

    // Details produit
    QFrame *details = makePanel();
    details->setMinimumHeight(215);
    QHBoxLayout *dh = new QHBoxLayout(details);
    dh->setContentsMargins(16, 14, 16, 14);

    QVBoxLayout *dv = new QVBoxLayout;
    dv->addWidget(makeTitle(QS("D\u00c9TAILS DU PRODUIT")));

    QGridLayout *dg = new QGridLayout;
    dg->setHorizontalSpacing(18);
    dg->setVerticalSpacing(2);

    auto addDetail = [dg](int r, int c, const QString &name, QLabel *&target)
    {
        QLabel *l = new QLabel(name);
        l->setObjectName("smallLabel");
        target = new QLabel(QS("--"));
        target->setObjectName("valueLabel");
        dg->addWidget(l, r * 2, c);
        dg->addWidget(target, r * 2 + 1, c);
    };

    addDetail(0, 0, QS("Nom"), dNom);
    addDetail(0, 1, QS("Type"), dType);
    addDetail(1, 0, QS("Prix unitaire"), dPrix);
    addDetail(1, 1, QS("Stock"), dStock);
    addDetail(2, 0, QS("Production"), dProd);
    addDetail(2, 1, QS("Expiration"), dExp);

    dv->addLayout(dg);
    dv->addStretch();
    dh->addLayout(dv, 1);

    BottleWidget *bottle = new BottleWidget;
    bottle->setFixedWidth(110);
    dh->addWidget(bottle, 0, Qt::AlignCenter);
    cards->addWidget(details, 1);

    // Niveau de stock
    QFrame *stock = makePanel();
    stock->setMinimumHeight(215);
    QVBoxLayout *sl = new QVBoxLayout(stock);
    sl->setContentsMargins(16, 14, 16, 14);
    sl->addWidget(makeTitle(QS("NIVEAU DE STOCK")));

    QHBoxLayout *sc = new QHBoxLayout;
    circle = new StockCircle;
    sc->addWidget(circle, 0, Qt::AlignCenter);

    QVBoxLayout *labels = new QVBoxLayout;
    labels->setSpacing(12);
    stFaible = new QLabel;
    stMoyen = new QLabel;
    stBon = new QLabel;
    stFaible->setStyleSheet(QS("color:#EF4444; font-weight:bold; font-size:12px;"));
    stMoyen->setStyleSheet(QS("color:#C48A00; font-weight:bold; font-size:12px;"));
    stBon->setStyleSheet(QS("color:#138A5B; font-weight:bold; font-size:12px;"));
    labels->addWidget(stFaible);
    labels->addWidget(stMoyen);
    labels->addWidget(stBon);
    labels->addStretch();
    sc->addLayout(labels);
    sl->addLayout(sc);
    cards->addWidget(stock, 1);

    // Repartition
    QFrame *prod = makePanel();
    prod->setMinimumHeight(215);
    QVBoxLayout *pl = new QVBoxLayout(prod);
    pl->setContentsMargins(16, 14, 16, 14);
    pl->addWidget(makeTitle(QS("R\u00c9PARTITION DE LA PRODUCTION")));

    const QStringList types = {tMin(), tGaz(), tAro()};
    for (int i = 0; i < 3; ++i)
    {
        QHBoxLayout *row = new QHBoxLayout;
        row->addWidget(new QLabel(types[i]));
        row->addStretch();

        pct[i] = new QLabel(QS("0%"));
        pct[i]->setObjectName("percent");
        row->addWidget(pct[i]);
        pl->addLayout(row);

        bars[i] = new QProgressBar;
        bars[i]->setRange(0, 100);
        bars[i]->setTextVisible(false);
        bars[i]->setFixedHeight(12);
        bars[i]->setStyleSheet(
            QS("QProgressBar { background:#E5EAF0; border:none; border-radius:5px; }"
               "QProgressBar::chunk { background:#2678E8; border-radius:5px; }"));
        pl->addWidget(bars[i]);
    }
    pl->addStretch();
    cards->addWidget(prod, 1);

    root->addLayout(cards);

    // ---------- Bas : formulaire + tableau ----------
    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->setSpacing(12);

    QFrame *form = makePanel();
    form->setMinimumWidth(430);
    form->setMaximumWidth(480);
    QVBoxLayout *fl = new QVBoxLayout(form);
    fl->setContentsMargins(16, 14, 16, 14);
    fl->setSpacing(8);

    QPushButton *clearButton = makeButton(QS("Effacer"), "secondary");
    clearButton->setStyleSheet(QS("padding:5px 14px;"));
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->addWidget(makeTitle(QS("\U0001F4E6  Ajouter / Modifier un produit")));
    titleRow->addStretch();
    titleRow->addWidget(clearButton);
    fl->addLayout(titleRow);

    idEdit = new QLineEdit;
    idEdit->setValidator(new QIntValidator(1, 999999, idEdit));

    nomEdit = new QLineEdit;
    nomEdit->setPlaceholderText(QS("Ex : Eau Cristal 2L"));

    typeCombo = new QComboBox;
    typeCombo->addItems({tMin(), tGaz(), tAro()});

    prixSpin = new QDoubleSpinBox;
    prixSpin->setRange(0, 999999);
    prixSpin->setDecimals(3);
    prixSpin->setSuffix(QS(" DT"));

    qteSpin = new QSpinBox;
    qteSpin->setRange(0, 9999999);

    prodEdit = makeDate(QDate::currentDate());
    expEdit = makeDate(QDate::currentDate().addYears(1));

    QGridLayout *grid = makeGrid(2);
    addField(grid, 0, 0, QS("ID produit"), idEdit);
    addField(grid, 0, 1, QS("Nom"), nomEdit);
    addField(grid, 1, 0, QS("Type"), typeCombo);
    addField(grid, 1, 1, QS("Prix unitaire"), prixSpin);
    addField(grid, 2, 0, QS("Quantit\u00e9 en stock"), qteSpin);
    addField(grid, 2, 1, QS("Date de production"), prodEdit);
    addField(grid, 3, 0, QS("Date d'expiration"), expEdit);
    fl->addLayout(grid);
    fl->addStretch();

    QPushButton *addButton = makeButton(QS("\uff0b  Ajouter"), "primary");
    QPushButton *modifyButton = makeButton(QS("\u270e  Modifier"), "warning");
    QPushButton *deleteButton = makeButton(QS("\U0001F5D1  Supprimer"), "danger");
    QPushButton *exportButton = makeButton(QS("\U0001F4C4  Exporter"), "secondary");
    addButton->setStyleSheet(QS("padding:10px 12px;"));
    modifyButton->setStyleSheet(QS("padding:10px 12px;"));
    deleteButton->setStyleSheet(QS("padding:10px 12px;"));
    exportButton->setStyleSheet(QS("padding:10px 12px;"));

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    buttons->addWidget(addButton);
    buttons->addWidget(modifyButton);
    buttons->addWidget(deleteButton);
    buttons->addWidget(exportButton);
    fl->addLayout(buttons);

    status = new StatusBox(QS("Cliquez sur un produit de la liste pour le modifier, "
                              "ou remplissez le formulaire puis cliquez sur \u00ab Ajouter \u00bb."));
    fl->addWidget(status);
    bottom->addWidget(form);

    QFrame *tablePanel = makePanel();
    QVBoxLayout *tl = new QVBoxLayout(tablePanel);
    tl->setContentsMargins(16, 14, 16, 14);
    tl->setSpacing(10);

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setPlaceholderText(QS("\U0001F50E  Rechercher (ID, nom, type)..."));
    searchEdit->setMaximumWidth(280);

    filtreCombo = new QComboBox;
    filtreCombo->addItems({QS("Tous les types"), tMin(), tGaz(), tAro()});
    filtreCombo->setMinimumWidth(150);

    QHBoxLayout *th = new QHBoxLayout;
    th->addWidget(makeTitle(QS("\U0001F4E6  Liste des produits")));
    th->addStretch();
    th->addWidget(searchEdit);
    th->addWidget(filtreCombo);
    tl->addLayout(th);

    table = makeTable({QS("ID"), QS("Nom"), QS("Type"), QS("Prix"), QS("Stock"),
                       QS("Date prod"), QS("Date exp")});
    table->setMinimumHeight(260);
    tl->addWidget(table, 1);
    bottom->addWidget(tablePanel, 1);

    root->addLayout(bottom, 1);

    // ---------- Connexions ----------
    connect(addButton, &QPushButton::clicked, this, [this]() { addItem(); });
    connect(modifyButton, &QPushButton::clicked, this, [this]() { modifyItem(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { removeItem(); });
    connect(exportButton, &QPushButton::clicked, this, [this]() { exportCsv(); });
    connect(clearButton, &QPushButton::clicked, this, [this]()
            {
                status->reset();
                clearForm();
            });
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { refresh(); });
    connect(filtreCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refresh(); });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this]()
            {
                const int id = rowId(table);
                if (id < 0)
                    return;

                status->cancel(true);

                if (const Produit *p = find(id))
                {
                    fillForm(*p);
                    showDetails(p);
                }
            });
}

void ProduitPage::refresh()
{
    table->setRowCount(0);

    const QString s = searchEdit->text().trimmed().toLower();
    const bool allTypes = filtreCombo->currentIndex() == 0;
    const QString type = filtreCombo->currentText();
    const QDate today = QDate::currentDate();

    for (const Produit &p : produits)
    {
        const bool okSearch = s.isEmpty()
        || code('P', p.id).toLower().contains(s)
            || p.nom.toLower().contains(s)
            || p.type.toLower().contains(s);

        if (!okSearch || (!allTypes && p.type != type))
            continue;

        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, idItem(code('P', p.id), p.id));
        table->setItem(row, 1, new QTableWidgetItem(p.nom));
        table->setItem(row, 2, new QTableWidgetItem(p.type));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(p.prix, 'f', 3) + QS(" DT")));

        QTableWidgetItem *st = new QTableWidgetItem(QString::number(p.stock));
        if (p.stock < 1000)
            boldRed(st);
        table->setItem(row, 4, st);

        table->setItem(row, 5, new QTableWidgetItem(fmtDate(p.dateProd)));

        QTableWidgetItem *ex = new QTableWidgetItem(fmtDate(p.dateExp));
        if (p.dateExp < today)
            boldRed(ex);
        table->setItem(row, 6, ex);
    }

    updatePanels();
    showDetails(find(rowId(table)));
}

void ProduitPage::showDetails(const Produit *p)
{
    if (!p)
    {
        dNom->setText(QS("Aucun produit s\u00e9lectionn\u00e9"));
        dType->setText(QS("--"));
        dPrix->setText(QS("--"));
        dStock->setText(QS("--"));
        dProd->setText(QS("--"));
        dExp->setText(QS("--"));
        return;
    }

    dNom->setText(p->nom);
    dType->setText(p->type);
    dPrix->setText(QString::number(p->prix, 'f', 3) + QS(" DT"));
    dStock->setText(QString::number(p->stock) + QS(" unit\u00e9s"));
    dProd->setText(fmtDate(p->dateProd));
    dExp->setText(fmtDate(p->dateExp));
}

void ProduitPage::updatePanels()
{
    int faible = 0, moyen = 0, bon = 0;
    double q[3] = {0, 0, 0};

    for (const Produit &p : produits)
    {
        if (p.stock < 1000)      ++faible;
        else if (p.stock < 3000) ++moyen;
        else                     ++bon;

        if (p.type == tMin())      q[0] += p.stock;
        else if (p.type == tGaz()) q[1] += p.stock;
        else                       q[2] += p.stock;
    }

    stFaible->setText(QS("\u25cf  Stock faible : %1").arg(faible));
    stMoyen->setText(QS("\u25cf  Stock moyen : %1").arg(moyen));
    stBon->setText(QS("\u25cf  Stock bon : %1").arg(bon));
    circle->setValues(faible, moyen, bon);

    const double total = q[0] + q[1] + q[2];
    int p1 = 0, p2 = 0, p3 = 0;

    if (total > 0)
    {
        p1 = qRound(q[0] * 100.0 / total);
        p2 = qRound(q[1] * 100.0 / total);
        p3 = 100 - p1 - p2;
    }

    const int values[3] = {p1, p2, p3};
    for (int i = 0; i < 3; ++i)
    {
        bars[i]->setValue(values[i]);
        pct[i]->setText(QString::number(values[i]) + QS("%"));
    }
}

void ProduitPage::clearForm()
{
    table->clearSelection();

    int next = 1;
    for (const Produit &p : produits)
        next = qMax(next, p.id + 1);

    idEdit->setText(QString::number(next));
    nomEdit->clear();
    typeCombo->setCurrentIndex(0);
    prixSpin->setValue(0);
    qteSpin->setValue(0);
    prodEdit->setDate(QDate::currentDate());
    expEdit->setDate(QDate::currentDate().addYears(1));
    showDetails(nullptr);
}

void ProduitPage::fillForm(const Produit &p)
{
    idEdit->setText(QString::number(p.id));
    nomEdit->setText(p.nom);

    const int index = typeCombo->findText(p.type);
    typeCombo->setCurrentIndex(index >= 0 ? index : 0);

    prixSpin->setValue(p.prix);
    qteSpin->setValue(p.stock);
    prodEdit->setDate(p.dateProd);
    expEdit->setDate(p.dateExp);
}

bool ProduitPage::readForm(Produit &out, bool isNew)
{
    bool ok = false;
    const int id = idEdit->text().trimmed().toInt(&ok);

    if (!ok || id <= 0)
    {
        status->say(QS("Veuillez saisir un ID produit valide."), StatusBox::Error);
        return false;
    }

    if (isNew && find(id))
    {
        status->say(QS("Cet ID produit existe d\u00e9j\u00e0."), StatusBox::Error);
        return false;
    }

    const QString nom = nomEdit->text().trimmed();
    if (nom.isEmpty())
    {
        status->say(QS("Le nom est obligatoire."), StatusBox::Error);
        return false;
    }

    if (expEdit->date() <= prodEdit->date())
    {
        status->say(QS("La date d'expiration doit \u00eatre apr\u00e8s la date de production."),
                    StatusBox::Error);
        return false;
    }

    out.id = id;
    out.nom = nom;
    out.type = typeCombo->currentText();
    out.prix = prixSpin->value();
    out.stock = qteSpin->value();
    out.dateProd = prodEdit->date();
    out.dateExp = expEdit->date();
    return true;
}

void ProduitPage::addItem()
{
    status->reset();

    Produit p{};
    if (!readForm(p, true))
        return;

    status->ask(QS("Ajouter \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(p.nom),
                [this, p]()
                {
                    produits.append(p);
                    refresh();
                    clearForm();
                    status->say(QS("Produit ajout\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ProduitPage::modifyItem()
{
    status->reset();

    const int id = rowId(table);
    if (id < 0)
    {
        status->say(QS("S\u00e9lectionnez d'abord un produit dans la liste."), StatusBox::Error);
        return;
    }

    Produit p{};
    if (!readForm(p, false))
        return;

    if (p.id != id)
    {
        status->say(QS("L'ID d'un produit ne peut pas \u00eatre modifi\u00e9."), StatusBox::Error);
        return;
    }

    status->ask(QS("Enregistrer les modifications de \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(p.nom),
                [this, p, id]()
                {
                    for (Produit &old : produits)
                        if (old.id == id)
                            old = p;

                    refresh();
                    selectById(table, id);
                    status->say(QS("Produit modifi\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ProduitPage::removeItem()
{
    status->reset();

    const int id = rowId(table);
    const Produit *p = find(id);
    if (!p)
    {
        status->say(QS("S\u00e9lectionnez d'abord un produit dans la liste."), StatusBox::Error);
        return;
    }

    status->ask(QS("Supprimer \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(p->nom),
                [this, id]()
                {
                    produits.erase(std::remove_if(produits.begin(), produits.end(),
                                                  [id](const Produit &x) { return x.id == id; }),
                                   produits.end());
                    refresh();
                    clearForm();
                    status->say(QS("Produit supprim\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ProduitPage::exportCsv()
{
    QStringList lines;
    for (const Produit &p : produits)
        lines << QS("%1;%2;%3;%4;%5;%6;%7")
                     .arg(code('P', p.id), csvq(p.nom), csvq(p.type),
                          QString::number(p.prix, 'f', 3), QString::number(p.stock),
                          fmtDate(p.dateProd), fmtDate(p.dateExp));

    exportTo(this, status, QS("produits"), QS("produits.csv"),
             QS("ID;Nom;Type;Prix;Stock;DateProduction;DateExpiration"), lines);
}

// ============================================================
// MODULE EQUIPEMENTS
// ============================================================

EquipementPage::EquipementPage(QWidget *parent)
    : Page(parent)
{
    load();
    build();

    alertTimer = new QTimer(this);
    connect(alertTimer, &QTimer::timeout, this, [this]() { refreshAlerts(true); });
    alertTimer->start(60000);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]()
            {
                blinkOn = !blinkOn;
                alertPanel->setStyleSheet(
                    QS("QFrame#alertPanel { background:white; border:2px solid %1; border-radius:10px; }")
                        .arg(blinkOn ? QS("#EF4444") : QS("#F7B4B4")));
            });

    refreshTable();
    updateStatistics();
    clearForm();
}

QString EquipementPage::summary() const
{
    int panne = 0;
    for (const Equipement &e : equipements)
        if (e.etat == etatPanne())
            ++panne;

    return QS("%1 \u00e9quipements \u2022 %2 en panne").arg(equipements.size()).arg(panne);
}

void EquipementPage::load()
{
    equipements.clear();
    const QDate t = QDate::currentDate();

    auto add = [this, t](int id, const QString &designation, const QString &type,
                         const QString &marque, const QString &modele, int installYearsAgo,
                         const QString &loc, const QString &etat, int usage,
                         int lastDaysAgo, int nextInDays, const QVector<int> &pannesDaysAgo)
    {
        QVector<QDate> pannes;
        for (int d : pannesDaysAgo)
            pannes.append(t.addDays(-d));

        equipements.append(Equipement{id, designation, type, marque, modele,
                                      t.addYears(-installYearsAgo), loc, etat, usage,
                                      t.addDays(-lastDaysAgo), t.addDays(nextInDays), pannes});
    };

    add(1, QS("Pompe centrifuge P-01"), QS("Pompe"), QS("Grundfos"), QS("CR 45"),
        4, QS("Atelier A"), etatOp(), 78, 50, 130, {300});
    add(2, QS("Filtre \u00e0 sable F-02"), QS("Filtre"), QS("Pall"), QS("SF-200"),
        5, QS("Station de filtration"), etatOp(), 55, 200, -20, {400});
    add(3, QS("Embouteilleuse EB-03"), QS("Embouteilleuse"), QS("Krones"), QS("Modulfill"),
        3, QS("Ligne 1"), etatPanne(), 92, 120, 30, {10, 70, 150, 260});
    add(4, QS("R\u00e9servoir inox R-04"), QS("R\u00e9servoir"), QS("Inoxtech"), QS("IT-10K"),
        6, QS("Zone de stockage"), etatOp(), 40, 90, 90, {});
    add(5, QS("Capteur pH C-05"), QS("Capteur"), QS("Endress+Hauser"), QS("CPS11D"),
        2, QS("Traitement"), etatMaint(), 65, 2, 178, {90, 200});
    add(6, QS("Compresseur CP-06"), QS("Compresseur"), QS("Atlas Copco"), QS("GA 30"),
        7, QS("Salle des machines"), etatOp(), 88, 230, -50, {30, 100, 190});
    add(7, QS("Pompe doseuse P-07"), QS("Pompe"), QS("Prominent"), QS("Beta"),
        1, QS("Traitement"), etatOp(), 35, 40, 140, {});
    add(8, QS("Capteur de d\u00e9bit C-08"), QS("Capteur"), QS("Siemens"), QS("Sitrans F M"),
        3, QS("Ligne 2"), etatPanne(), 70, 140, -5, {5, 250});
}

const Equipement *EquipementPage::find(int id) const
{
    for (const Equipement &e : equipements)
        if (e.id == id)
            return &e;
    return nullptr;
}

void EquipementPage::build()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 16, 22, 16);
    root->setSpacing(12);

    root->addWidget(makeHeader(QS("\U0001F6E0  Gestion des \u00c9quipements"),
                               QS("Ajoutez, modifiez et suivez les \u00e9quipements de l'usine HydroConnect")));

    // ---------- Cartes ----------
    QHBoxLayout *cards = new QHBoxLayout;
    cards->setSpacing(12);
    cards->addWidget(makeCard(QS("Total \u00e9quipements"), QS("\U0001F6E0"), QS("#E3F0FF"), &totalLabel));
    cards->addWidget(makeCard(QS("Op\u00e9rationnels"), QS("\u2705"), QS("#E3F8EF"), &opLabel));
    cards->addWidget(makeCard(QS("En panne"), QS("\U0001F6A8"), QS("#FDE8E8"), &panneLabel));
    cards->addWidget(makeCard(QS("En maintenance"), QS("\U0001F527"), QS("#FFF0E0"), &maintLabel));
    cards->addWidget(makeCard(QS("Taux de disponibilit\u00e9"), QS("\U0001F4C8"), QS("#EFE8FF"), &dispoLabel));
    root->addLayout(cards);

    // ---------- Tableau ----------
    QFrame *tablePanel = makePanel();
    QVBoxLayout *tl = new QVBoxLayout(tablePanel);
    tl->setContentsMargins(16, 14, 16, 14);
    tl->setSpacing(10);
    tl->addWidget(makeTitle(QS("\U0001F6E0  Liste des \u00e9quipements")));

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setPlaceholderText(QS("\U0001F50E  Rechercher (ID, nom, type, localisation ou \u00e9tat)..."));

    triCombo = new QComboBox;
    triCombo->addItems({QS("Aucun"), QS("\u00c9tat"), QS("Type"), QS("Date d'installation"),
                        QS("Niveau d'utilisation")});
    triCombo->setMinimumWidth(150);

    etatFilter = new QComboBox;
    etatFilter->addItems({QS("Tous"), etatOp(), etatPanne(), etatMaint()});
    etatFilter->setMinimumWidth(130);

    QHBoxLayout *sl = new QHBoxLayout;
    sl->setSpacing(10);
    sl->addWidget(searchEdit, 1);
    sl->addWidget(new QLabel(QS("Trier par :")));
    sl->addWidget(triCombo);
    sl->addWidget(new QLabel(QS("\u00c9tat :")));
    sl->addWidget(etatFilter);
    tl->addLayout(sl);

    table = makeTable({QS("ID"), QS("D\u00e9signation"), QS("Type"), QS("Localisation"),
                       QS("Installation"), QS("\u00c9tat"), QS("Utilisation"), QS("Proch. maint.")});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    table->setColumnWidth(5, 125);
    tl->addWidget(table, 1);

    QPushButton *addButton = makeButton(QS("\uff0b  Ajouter"), "primary");
    QPushButton *modifyButton = makeButton(QS("\u270e  Modifier"), "warning");
    QPushButton *deleteButton = makeButton(QS("\U0001F5D1  Supprimer"), "danger");
    QPushButton *exportButton = makeButton(QS("\U0001F4C4  Exporter Excel"), "secondary");

    QHBoxLayout *bl = new QHBoxLayout;
    bl->setSpacing(12);
    bl->addWidget(addButton);
    bl->addWidget(modifyButton);
    bl->addWidget(deleteButton);
    bl->addWidget(exportButton);
    bl->addStretch();
    tl->addLayout(bl);

    // ---------- Statistiques ----------
    QFrame *statPanel = makePanel();
    QVBoxLayout *stl = new QVBoxLayout(statPanel);
    stl->setContentsMargins(16, 14, 16, 14);
    stl->setSpacing(10);
    stl->addWidget(makeTitle(QS("\U0001F4CA  Statistiques \u00e9quipements")));

    QFrame *barCard = new QFrame;
    barCard->setObjectName("innerCard");
    QVBoxLayout *bcl = new QVBoxLayout(barCard);
    bcl->setContentsMargins(2, 2, 2, 2);
    barChart = new BarChart;
    barChart->setTitle(QS("Fr\u00e9quence des pannes (6 derniers mois)"));
    barChart->setColors({QColor("#EF4444")});
    bcl->addWidget(barChart);

    QFrame *pieCard = new QFrame;
    pieCard->setObjectName("innerCard");
    QVBoxLayout *pcl = new QVBoxLayout(pieCard);
    pcl->setContentsMargins(2, 2, 2, 2);
    pieChart = new PieChart;
    pieChart->setTitle(QS("R\u00e9partition des \u00e9quipements par \u00e9tat"));
    pieChart->setColors({QColor("#20B486"), QColor("#EF4444"), QColor("#FF8A34")});
    pcl->addWidget(pieChart);

    stl->addWidget(barCard, 1);
    stl->addWidget(pieCard, 1);

    QHBoxLayout *center = new QHBoxLayout;
    center->setSpacing(12);
    center->addWidget(tablePanel, 11);
    center->addWidget(statPanel, 5);
    root->addLayout(center, 5);

    // ---------- Bas : prediction + alertes + formulaire ----------
    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->setSpacing(12);

    QFrame *predict = makePanel();
    QVBoxLayout *pl = new QVBoxLayout(predict);
    pl->setContentsMargins(16, 14, 16, 14);
    pl->setSpacing(12);
    pl->addWidget(makeTitle(QS("\U0001F916  Maintenance pr\u00e9dictive (IA)")));

    QFormLayout *pf = new QFormLayout;
    pf->setHorizontalSpacing(14);
    pf->setVerticalSpacing(12);
    pf->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    predictCombo = new QComboBox;
    pf->addRow(QS("\u00c9quipement :"), predictCombo);

    riskBar = new QProgressBar;
    riskBar->setRange(0, 100);
    riskBar->setTextVisible(false);
    riskBar->setFixedHeight(10);
    riskLabel = new QLabel(QS("-"));
    riskLabel->setObjectName("scoreText");
    QHBoxLayout *rr = new QHBoxLayout;
    rr->addWidget(riskBar, 1);
    rr->addWidget(riskLabel);
    pf->addRow(QS("Score de risque :"), rr);

    riskLevelLabel = new QLabel(QS("-"));
    riskLevelLabel->setAlignment(Qt::AlignCenter);
    riskLevelLabel->setMinimumWidth(90);
    riskLevelLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    pf->addRow(QS("Niveau de risque :"), riskLevelLabel);

    predictedDateLabel = new QLabel(QS("-"));
    predictedDateLabel->setObjectName("pillBlue");
    predictedDateLabel->setAlignment(Qt::AlignCenter);
    pf->addRow(QS("Maintenance conseill\u00e9e :"), predictedDateLabel);
    pl->addLayout(pf);

    predictedInfoLabel = new QLabel;
    predictedInfoLabel->setWordWrap(true);
    predictedInfoLabel->setStyleSheet(QS("font-size:12px; color:#526173;"));
    pl->addWidget(predictedInfoLabel);
    pl->addStretch();

    QPushButton *predictButton = makeButton(QS("Analyser l'\u00e9quipement"), "primary");
    pl->addWidget(predictButton, 0, Qt::AlignHCenter);

    alertPanel = new QFrame;
    alertPanel->setObjectName("alertPanel");
    QVBoxLayout *al = new QVBoxLayout(alertPanel);
    al->setContentsMargins(16, 14, 16, 14);
    al->setSpacing(10);

    alertBadge = new QLabel(QS("Aucune alerte"));
    alertBadge->setAlignment(Qt::AlignCenter);
    QPushButton *checkButton = makeButton(QS("V\u00e9rifier"), "secondary");
    checkButton->setStyleSheet(QS("padding:5px 14px;"));

    QHBoxLayout *ath = new QHBoxLayout;
    ath->addWidget(makeTitle(QS("\U0001F514  Smart Alert")));
    ath->addStretch();
    ath->addWidget(alertBadge);
    ath->addWidget(checkButton);
    al->addLayout(ath);

    alertList = new QListWidget;
    alertList->setSelectionMode(QAbstractItemView::NoSelection);
    alertList->setWordWrap(true);
    al->addWidget(alertList, 1);

    bottom->addWidget(predict, 9);
    bottom->addWidget(alertPanel, 10);
    bottom->addWidget(buildForm(), 15);
    root->addLayout(bottom, 3);

    // ---------- Connexions ----------
    connect(addButton, &QPushButton::clicked, this, [this]() { addItem(); });
    connect(modifyButton, &QPushButton::clicked, this, [this]() { modifyItem(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { removeItem(); });
    connect(exportButton, &QPushButton::clicked, this, [this]() { exportCsv(); });
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { refreshTable(); });
    connect(etatFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(predictButton, &QPushButton::clicked, this, [this]() { runPrediction(); });
    connect(predictCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { runPrediction(); });
    connect(checkButton, &QPushButton::clicked, this, [this]()
            {
                refreshAlerts(false);
                if (!buildAlerts().isEmpty())
                    QApplication::beep();
            });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this]()
            {
                const int id = rowId(table);
                if (id < 0)
                    return;

                status->cancel(true);

                if (const Equipement *e = find(id))
                    fillForm(*e);
            });
}

QWidget *EquipementPage::buildForm()
{
    QFrame *panel = makePanel();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(8);

    QPushButton *clearButton = makeButton(QS("Effacer"), "secondary");
    clearButton->setStyleSheet(QS("padding:5px 14px;"));
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->addWidget(makeTitle(QS("\U0001F6E0  Ajouter / Modifier un \u00e9quipement")));
    titleRow->addStretch();
    titleRow->addWidget(clearButton);
    layout->addLayout(titleRow);

    idEdit = new QLineEdit;
    idEdit->setValidator(new QIntValidator(1, 999999, idEdit));

    designationEdit = new QLineEdit;
    designationEdit->setPlaceholderText(QS("Ex : Pompe centrifuge P-01"));

    typeCombo = new QComboBox;
    typeCombo->addItems({QS("Pompe"), QS("Filtre"), QS("R\u00e9servoir"), QS("Embouteilleuse"),
                         QS("Capteur"), QS("Compresseur"), QS("Autre")});

    marqueEdit = new QLineEdit;
    marqueEdit->setPlaceholderText(QS("Ex : Grundfos"));
    modeleEdit = new QLineEdit;
    modeleEdit->setPlaceholderText(QS("Ex : CR 45"));
    localisationEdit = new QLineEdit;
    localisationEdit->setPlaceholderText(QS("Ex : Atelier A"));

    etatCombo = new QComboBox;
    etatCombo->addItems({etatOp(), etatPanne(), etatMaint()});

    utilisationSpin = new QSpinBox;
    utilisationSpin->setRange(0, 100);
    utilisationSpin->setSuffix(QS(" %"));

    installationEdit = makeDate(QDate::currentDate());
    derniereMaintEdit = makeDate(QDate::currentDate());
    prochaineMaintEdit = makeDate(QDate::currentDate());

    QGridLayout *grid = makeGrid(3);
    addField(grid, 0, 0, QS("ID \u00e9quipement"), idEdit);
    addField(grid, 0, 1, QS("D\u00e9signation"), designationEdit);
    addField(grid, 0, 2, QS("Type"), typeCombo);
    addField(grid, 1, 0, QS("Marque"), marqueEdit);
    addField(grid, 1, 1, QS("Mod\u00e8le"), modeleEdit);
    addField(grid, 1, 2, QS("Localisation"), localisationEdit);
    addField(grid, 2, 0, QS("\u00c9tat"), etatCombo);
    addField(grid, 2, 1, QS("Niveau d'utilisation"), utilisationSpin);
    addField(grid, 2, 2, QS("Date d'installation"), installationEdit);
    addField(grid, 3, 0, QS("Derni\u00e8re maintenance"), derniereMaintEdit);
    addField(grid, 3, 1, QS("Prochaine maintenance"), prochaineMaintEdit);
    layout->addLayout(grid);
    layout->addStretch();

    status = new StatusBox(QS("Cliquez sur un \u00e9quipement de la liste pour le modifier, "
                              "ou remplissez le formulaire puis cliquez sur \u00ab Ajouter \u00bb."));
    layout->addWidget(status);

    connect(clearButton, &QPushButton::clicked, this, [this]()
            {
                status->reset();
                clearForm();
            });

    return panel;
}

void EquipementPage::refreshTable()
{
    table->setRowCount(0);

    const QString search = searchEdit->text().trimmed().toLower();
    const QString etatSel = etatFilter->currentText();
    const bool allEtats = etatFilter->currentIndex() == 0;
    const int sortMode = triCombo->currentIndex();

    auto rank = [](const QString &etat)
    {
        if (etat == etatPanne()) return 0;
        if (etat == etatMaint()) return 1;
        return 2;
    };

    QVector<Equipement> list = equipements;
    std::stable_sort(list.begin(), list.end(),
                     [sortMode, &rank](const Equipement &a, const Equipement &b)
                     {
                         switch (sortMode)
                         {
                         case 1:
                             if (rank(a.etat) != rank(b.etat)) return rank(a.etat) < rank(b.etat);
                             break;
                         case 2:
                             if (a.type != b.type) return a.type < b.type;
                             break;
                         case 3:
                             if (a.dateInstallation != b.dateInstallation)
                                 return a.dateInstallation < b.dateInstallation;
                             break;
                         case 4:
                             if (a.niveauUtilisation != b.niveauUtilisation)
                                 return a.niveauUtilisation > b.niveauUtilisation;
                             break;
                         default:
                             break;
                         }
                         return a.id < b.id;
                     });

    const QDate today = QDate::currentDate();

    for (const Equipement &e : list)
    {
        const bool okSearch = search.isEmpty()
        || code('E', e.id).toLower().contains(search)
            || e.designation.toLower().contains(search)
            || e.type.toLower().contains(search)
            || e.localisation.toLower().contains(search)
            || e.etat.toLower().contains(search);

        if (!okSearch || (!allEtats && e.etat != etatSel))
            continue;

        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, idItem(code('E', e.id), e.id));

        QTableWidgetItem *name = new QTableWidgetItem(e.designation);
        name->setToolTip(QS("%1 %2").arg(e.marque, e.modele));
        table->setItem(row, 1, name);
        table->setItem(row, 2, new QTableWidgetItem(e.type));
        table->setItem(row, 3, new QTableWidgetItem(e.localisation));
        table->setItem(row, 4, new QTableWidgetItem(fmtDate(e.dateInstallation)));
        table->setItem(row, 5, new QTableWidgetItem(QString()));

        QTableWidgetItem *use = new QTableWidgetItem(QS("%1 %").arg(e.niveauUtilisation));
        use->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (e.niveauUtilisation >= 90)
            boldRed(use);
        table->setItem(row, 6, use);

        const bool overdue = e.prochaineMaintenance.isValid() && e.prochaineMaintenance < today;
        QTableWidgetItem *next = new QTableWidgetItem(
            fmtDate(e.prochaineMaintenance) + (overdue ? QS("  \u26A0") : QString()));
        next->setToolTip(QS("Derni\u00e8re maintenance : %1").arg(fmtDate(e.derniereMaintenance)));
        if (overdue)
            boldRed(next);
        table->setItem(row, 7, next);

        QString bg, fg;
        if (e.etat == etatOp())         { bg = QS("#D9FBEA"); fg = QS("#138A5B"); }
        else if (e.etat == etatPanne()) { bg = QS("#FDE8E8"); fg = QS("#B42318"); }
        else                            { bg = QS("#FFE9D2"); fg = QS("#C45B00"); }
        setPill(table, row, 5, e.etat, bg, fg);
    }
}

void EquipementPage::updateStatistics()
{
    int op = 0, panne = 0, maint = 0;

    for (const Equipement &e : equipements)
    {
        if (e.etat == etatOp())         ++op;
        else if (e.etat == etatPanne()) ++panne;
        else                            ++maint;
    }

    const int total = equipements.size();
    const double dispo = total > 0 ? op * 100.0 / total : 0.0;

    totalLabel->setText(QString::number(total));
    opLabel->setText(QString::number(op));
    panneLabel->setText(QString::number(panne));
    maintLabel->setText(QString::number(maint));
    dispoLabel->setText(QS("%1 %").arg(QString::number(dispo, 'f', 1)));

    const QDate today = QDate::currentDate();
    const QLocale fr(QLocale::French);

    QVector<double> counts(6, 0.0);
    QVector<QString> labels;

    for (int i = 0; i < 6; ++i)
    {
        const QDate month = today.addMonths(-(5 - i));
        labels.append(fr.monthName(month.month(), QLocale::ShortFormat));

        for (const Equipement &e : equipements)
            for (const QDate &d : e.historiquePannes)
                if (d.year() == month.year() && d.month() == month.month())
                    counts[i] += 1.0;
    }

    barChart->setData(counts, labels);
    pieChart->setData({double(op), double(panne), double(maint)},
                      {QS("Op\u00e9rationnels"), QS("En panne"), QS("En maintenance")});

    updatePredictCombo();
    runPrediction();
    refreshAlerts(true);
}

void EquipementPage::clearForm()
{
    table->clearSelection();

    int next = 1;
    for (const Equipement &e : equipements)
        next = qMax(next, e.id + 1);

    const QDate today = QDate::currentDate();

    idEdit->setText(QString::number(next));
    designationEdit->clear();
    typeCombo->setCurrentIndex(0);
    marqueEdit->clear();
    modeleEdit->clear();
    localisationEdit->clear();
    etatCombo->setCurrentIndex(0);
    utilisationSpin->setValue(0);
    installationEdit->setDate(today);
    derniereMaintEdit->setDate(today);
    prochaineMaintEdit->setDate(today.addMonths(6));
}

void EquipementPage::fillForm(const Equipement &e)
{
    idEdit->setText(QString::number(e.id));
    designationEdit->setText(e.designation);

    int index = typeCombo->findText(e.type);
    typeCombo->setCurrentIndex(index >= 0 ? index : 0);

    marqueEdit->setText(e.marque);
    modeleEdit->setText(e.modele);
    localisationEdit->setText(e.localisation);

    index = etatCombo->findText(e.etat);
    etatCombo->setCurrentIndex(index >= 0 ? index : 0);

    utilisationSpin->setValue(e.niveauUtilisation);
    installationEdit->setDate(e.dateInstallation);
    derniereMaintEdit->setDate(e.derniereMaintenance);
    prochaineMaintEdit->setDate(e.prochaineMaintenance);
}

bool EquipementPage::readForm(Equipement &out, bool isNew)
{
    bool ok = false;
    const int id = idEdit->text().trimmed().toInt(&ok);

    if (!ok || id <= 0)
    {
        status->say(QS("Veuillez saisir un ID \u00e9quipement valide."), StatusBox::Error);
        return false;
    }

    if (isNew && find(id))
    {
        status->say(QS("Cet ID \u00e9quipement existe d\u00e9j\u00e0."), StatusBox::Error);
        return false;
    }

    const QString designation = designationEdit->text().trimmed();
    const QString marque = marqueEdit->text().trimmed();
    const QString modele = modeleEdit->text().trimmed();
    const QString localisation = localisationEdit->text().trimmed();

    if (designation.isEmpty())
    {
        status->say(QS("La d\u00e9signation est obligatoire."), StatusBox::Error);
        return false;
    }

    if (marque.isEmpty() || modele.isEmpty())
    {
        status->say(QS("La marque et le mod\u00e8le sont obligatoires."), StatusBox::Error);
        return false;
    }

    if (localisation.isEmpty())
    {
        status->say(QS("La localisation est obligatoire."), StatusBox::Error);
        return false;
    }

    const QDate install = installationEdit->date();
    const QDate last = derniereMaintEdit->date();
    const QDate next = prochaineMaintEdit->date();

    if (install > QDate::currentDate())
    {
        status->say(QS("La date d'installation ne peut pas \u00eatre dans le futur."), StatusBox::Error);
        return false;
    }

    if (last < install)
    {
        status->say(QS("La derni\u00e8re maintenance doit \u00eatre apr\u00e8s l'installation."), StatusBox::Error);
        return false;
    }

    if (next < last)
    {
        status->say(QS("La prochaine maintenance doit \u00eatre apr\u00e8s la derni\u00e8re."), StatusBox::Error);
        return false;
    }

    out.id = id;
    out.designation = designation;
    out.type = typeCombo->currentText();
    out.marque = marque;
    out.modele = modele;
    out.dateInstallation = install;
    out.localisation = localisation;
    out.etat = etatCombo->currentText();
    out.niveauUtilisation = utilisationSpin->value();
    out.derniereMaintenance = last;
    out.prochaineMaintenance = next;
    return true;
}

void EquipementPage::addItem()
{
    status->reset();

    Equipement e{};
    if (!readForm(e, true))
        return;

    if (e.etat == etatPanne())
        e.historiquePannes.append(QDate::currentDate());

    status->ask(QS("Ajouter \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(e.designation),
                [this, e]()
                {
                    equipements.append(e);
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("\u00c9quipement ajout\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EquipementPage::modifyItem()
{
    status->reset();

    const int id = rowId(table);
    if (id < 0)
    {
        status->say(QS("S\u00e9lectionnez d'abord un \u00e9quipement dans la liste."), StatusBox::Error);
        return;
    }

    Equipement e{};
    if (!readForm(e, false))
        return;

    if (e.id != id)
    {
        status->say(QS("L'ID d'un \u00e9quipement ne peut pas \u00eatre modifi\u00e9."), StatusBox::Error);
        return;
    }

    if (const Equipement *old = find(id))
    {
        e.historiquePannes = old->historiquePannes;
        if (old->etat != etatPanne() && e.etat == etatPanne())
            e.historiquePannes.append(QDate::currentDate());
    }

    status->ask(QS("Enregistrer les modifications de \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler")
                    .arg(e.designation),
                [this, e, id]()
                {
                    for (Equipement &old : equipements)
                        if (old.id == id)
                            old = e;

                    refreshTable();
                    updateStatistics();
                    selectById(table, id);
                    status->say(QS("\u00c9quipement modifi\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EquipementPage::removeItem()
{
    status->reset();

    const int id = rowId(table);
    const Equipement *e = find(id);
    if (!e)
    {
        status->say(QS("S\u00e9lectionnez d'abord un \u00e9quipement dans la liste."), StatusBox::Error);
        return;
    }

    status->ask(QS("Supprimer \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(e->designation),
                [this, id]()
                {
                    equipements.erase(std::remove_if(equipements.begin(), equipements.end(),
                                                     [id](const Equipement &x) { return x.id == id; }),
                                      equipements.end());
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("\u00c9quipement supprim\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EquipementPage::exportCsv()
{
    QStringList lines;
    for (const Equipement &e : equipements)
        lines << QS("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12")
                     .arg(code('E', e.id), csvq(e.designation), csvq(e.type), csvq(e.marque),
                          csvq(e.modele), fmtDate(e.dateInstallation), csvq(e.localisation),
                          csvq(e.etat), QString::number(e.niveauUtilisation),
                          fmtDate(e.derniereMaintenance))
                     .arg(fmtDate(e.prochaineMaintenance))
                     .arg(e.historiquePannes.size());

    exportTo(this, status, QS("\u00e9quipements"), QS("equipements.csv"),
             QS("ID;Designation;Type;Marque;Modele;DateInstallation;Localisation;Etat;"
                "NiveauUtilisation;DerniereMaintenance;ProchaineMaintenance;NbPannes"),
             lines);
}

RiskResult EquipementPage::analyseRisk(const Equipement &e) const
{
    const QDate today = QDate::currentDate();
    RiskResult r;

    for (const QDate &d : e.historiquePannes)
    {
        const int age = d.daysTo(today);
        if (age >= 0 && age <= 365)
            ++r.count12m;
    }

    double score = 0;
    score += qMin(r.count12m * 15.0, 45.0);
    score += e.niveauUtilisation * 0.25;

    const int sinceLast = e.derniereMaintenance.isValid() ? e.derniereMaintenance.daysTo(today) : 9999;

    if (e.prochaineMaintenance.isValid() && e.prochaineMaintenance < today)
        score += 20;
    else if (sinceLast > 150)
        score += 10;

    const int ageYears = e.dateInstallation.isValid() ? e.dateInstallation.daysTo(today) / 365 : 0;
    score += qMin(ageYears * 1.5, 10.0);

    r.score = qBound(0, qRound(score), 100);

    if (r.score >= 70)      r.niveau = QS("\u00c9lev\u00e9");
    else if (r.score >= 40) r.niveau = QS("Moyen");
    else                    r.niveau = QS("Faible");

    // plus le risque est haut, plus l'intervalle est court (30..180 jours)
    const int interval = qBound(30, 180 - qRound(r.score * 1.5), 180);

    QDate proposed = e.derniereMaintenance.isValid() ? e.derniereMaintenance.addDays(interval)
                                                     : today.addDays(7);

    // au moins 2 pannes : MTBF -> maintenance 7 jours avant la prochaine panne estimee
    if (e.historiquePannes.size() >= 2)
    {
        QVector<QDate> sorted = e.historiquePannes;
        std::sort(sorted.begin(), sorted.end());

        const int mtbf = sorted.first().daysTo(sorted.last()) / (sorted.size() - 1);
        const QDate guard = sorted.last().addDays(mtbf - 7);

        if (guard < proposed)
            proposed = guard;
    }

    if (e.etat == etatPanne())
        proposed = today;
    else if (proposed < today)
        proposed = today.addDays(7);

    r.dateProposee = proposed;
    return r;
}

void EquipementPage::updatePredictCombo()
{
    QVector<QPair<QString, int>> items;
    for (const Equipement &e : equipements)
        items.append(qMakePair(code('E', e.id) + QS(" - ") + e.designation, e.id));

    fillIdCombo(predictCombo, items);
}

void EquipementPage::runPrediction()
{
    if (predictCombo->currentIndex() < 0)
    {
        riskBar->setValue(0);
        riskLabel->setText(QS("-"));
        riskLevelLabel->setText(QS("-"));
        predictedDateLabel->setText(QS("-"));
        predictedInfoLabel->clear();
        return;
    }

    const Equipement *e = find(predictCombo->currentData().toInt());
    if (!e)
        return;

    const RiskResult r = analyseRisk(*e);

    applyRisk(riskBar, riskLevelLabel, r.score);
    riskLabel->setText(QS("%1 / 100").arg(r.score));
    riskLevelLabel->setText(r.niveau);
    predictedDateLabel->setText(fmtDate(r.dateProposee));

    predictedInfoLabel->setText(
        QS("%1 panne(s) sur 12 mois  \u2022  utilisation %2 %  \u2022  derni\u00e8re maintenance : %3")
            .arg(r.count12m)
            .arg(e->niveauUtilisation)
            .arg(fmtDate(e->derniereMaintenance)));
}

QVector<AlertItem> EquipementPage::buildAlerts() const
{
    const QDate today = QDate::currentDate();
    QVector<AlertItem> alerts;

    for (const Equipement &e : equipements)
    {
        const QString name = code('E', e.id) + QS(" - ") + e.designation;

        if (e.etat == etatPanne())
            alerts.append({0, QS("\U0001F534  %1 : \u00e9quipement en panne").arg(name)});

        if (e.prochaineMaintenance.isValid() && e.prochaineMaintenance < today)
            alerts.append({1, QS("\U0001F7E0  %1 : maintenance d\u00e9pass\u00e9e de %2 jour(s)")
                                  .arg(name).arg(e.prochaineMaintenance.daysTo(today))});

        const RiskResult r = analyseRisk(e);
        if (r.score >= 70)
            alerts.append({2, QS("\u26A0  %1 : risque de panne \u00e9lev\u00e9 (%2/100)")
                                  .arg(name).arg(r.score)});
    }

    std::stable_sort(alerts.begin(), alerts.end(),
                     [](const AlertItem &a, const AlertItem &b) { return a.level < b.level; });
    return alerts;
}

void EquipementPage::refreshAlerts(bool allowSound)
{
    const QVector<AlertItem> alerts = buildAlerts();
    alertList->clear();

    for (const AlertItem &a : alerts)
    {
        QListWidgetItem *item = new QListWidgetItem(a.text);
        item->setForeground(alertColor(a.level));
        alertList->addItem(item);
    }

    if (alerts.isEmpty())
    {
        alertList->addItem(QS("\u2705  Aucune alerte : tous les \u00e9quipements sont OK."));
        alertBadge->setText(QS("Aucune alerte"));
        alertBadge->setStyleSheet(QS("background:#D9FBEA; color:#138A5B; border-radius:6px;"
                                     "padding:5px 12px; font-weight:bold; font-size:12px;"));
        blinkTimer->stop();
        alertPanel->setStyleSheet(QS("QFrame#alertPanel { background:white;"
                                     "border:1px solid #DFE6EF; border-radius:10px; }"));
    }
    else
    {
        alertBadge->setText(QS("%1 alerte(s)").arg(alerts.size()));
        alertBadge->setStyleSheet(QS("background:#EF4444; color:white; border-radius:6px;"
                                     "padding:5px 12px; font-weight:bold; font-size:12px;"));

        if (!blinkTimer->isActive())
            blinkTimer->start(700);

        if (allowSound && alerts.size() > lastAlertCount)
            QApplication::beep();
    }

    lastAlertCount = alerts.size();
}

// ============================================================
// MODULE CLIENTS
// ============================================================

ClientPage::ClientPage(QWidget *parent)
    : Page(parent)
{
    load();
    build();
    refreshTable();
    updateStatistics();
    clearForm();
}

QString ClientPage::summary() const
{
    double ca = 0;
    for (const Client &c : clients)
        ca += c.chiffreAffaires;

    return QS("%1 clients \u2022 CA %2 DT").arg(clients.size()).arg(spaced(ca));
}

void ClientPage::load()
{
    clients.clear();
    clients.append(Client{1, QS("SONEDE"), QS("contact@sonede.tn"), QS("71 123 456"), catEntreprise(), 85000});
    clients.append(Client{2, QS("Aqua Plus"), QS("aqua@plus.tn"), QS("98 765 432"), catGrossiste(), 62500});
    clients.append(Client{3, QS("Eau Pure"), QS("eaupure@gmail.com"), QS("22 345 678"), catDetaillant(), 35200});
    clients.append(Client{4, QS("Hydro Tunisie"), QS("contact@hydro.tn"), QS("55 987 654"), catEntreprise(), 29800});
    clients.append(Client{5, QS("Crystal Water"), QS("crystal@water.tn"), QS("23 456 789"), catGrossiste(), 28400});
    clients.append(Client{6, QS("Blue Wave"), QS("blue@wave.tn"), QS("96 321 654"), catDetaillant(), 22700});
    clients.append(Client{7, QS("Fresh Water"), QS("fresh@gmail.com"), QS("27 654 321"), catDetaillant(), 19300});
    clients.append(Client{8, QS("Aqua Distribution"), QS("contact@aqua.tn"), QS("52 963 741"), catGrossiste(), 18600});
}

const Client *ClientPage::find(int id) const
{
    for (const Client &c : clients)
        if (c.id == id)
            return &c;
    return nullptr;
}

void ClientPage::build()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 16, 22, 16);
    root->setSpacing(12);

    root->addWidget(makeHeader(QS("\U0001F465  Gestion des Clients"),
                               QS("Ajoutez, modifiez et suivez vos clients de HydroConnect")));

    QHBoxLayout *cards = new QHBoxLayout;
    cards->setSpacing(12);
    cards->addWidget(makeCard(QS("Nombre de clients"), QS("\U0001F465"), QS("#E3F0FF"), &nombreLabel));
    cards->addWidget(makeCard(QS("Chiffre d'affaires total"), QS("\U0001F4C8"), QS("#E3F8EF"), &caTotalLabel));
    cards->addWidget(makeCard(QS("Grossistes"), QS("\U0001F6D2"), QS("#FFF0E0"), &grossistesLabel));
    cards->addWidget(makeCard(QS("D\u00e9taillants"), QS("\U0001F3EA"), QS("#EFE8FF"), &detaillantsLabel));
    cards->addWidget(makeCard(QS("Entreprises"), QS("\U0001F3E2"), QS("#E3F0FF"), &entreprisesLabel));
    root->addLayout(cards);

    // ---------- Tableau ----------
    QFrame *tablePanel = makePanel();
    QVBoxLayout *tl = new QVBoxLayout(tablePanel);
    tl->setContentsMargins(16, 14, 16, 14);
    tl->setSpacing(10);
    tl->addWidget(makeTitle(QS("\U0001F465  Liste des clients")));

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setPlaceholderText(QS("\U0001F50E  Rechercher un client (nom, email ou t\u00e9l\u00e9phone)..."));

    triCombo = new QComboBox;
    triCombo->addItems({QS("Aucun"), QS("Chiffre d'affaires"), QS("Cat\u00e9gorie")});
    triCombo->setMinimumWidth(150);

    categorieFilter = new QComboBox;
    categorieFilter->addItems({QS("Toutes"), catGrossiste(), catDetaillant(), catEntreprise()});
    categorieFilter->setMinimumWidth(120);

    QHBoxLayout *sl = new QHBoxLayout;
    sl->setSpacing(10);
    sl->addWidget(searchEdit, 1);
    sl->addWidget(new QLabel(QS("Trier par :")));
    sl->addWidget(triCombo);
    sl->addWidget(new QLabel(QS("Cat\u00e9gorie :")));
    sl->addWidget(categorieFilter);
    tl->addLayout(sl);

    table = makeTable({QS("ID Client"), QS("Nom / Raison Sociale"), QS("Email"), QS("T\u00e9l\u00e9phone"),
                       QS("Cat\u00e9gorie"), QS("Chiffre d'affaires (DT)")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tl->addWidget(table, 1);

    QPushButton *addButton = makeButton(QS("\uff0b  Ajouter"), "primary");
    QPushButton *modifyButton = makeButton(QS("\u270e  Modifier"), "warning");
    QPushButton *deleteButton = makeButton(QS("\U0001F5D1  Supprimer"), "danger");
    QPushButton *exportButton = makeButton(QS("\U0001F4C4  Exporter"), "secondary");

    QHBoxLayout *bl = new QHBoxLayout;
    bl->setSpacing(12);
    bl->addWidget(addButton);
    bl->addWidget(modifyButton);
    bl->addWidget(deleteButton);
    bl->addWidget(exportButton);
    bl->addStretch();
    tl->addLayout(bl);

    // ---------- Statistiques ----------
    QFrame *statPanel = makePanel();
    QVBoxLayout *stl = new QVBoxLayout(statPanel);
    stl->setContentsMargins(16, 14, 16, 14);
    stl->setSpacing(10);
    stl->addWidget(makeTitle(QS("\U0001F4CA  Statistiques clients")));

    QFrame *barCard = new QFrame;
    barCard->setObjectName("innerCard");
    QVBoxLayout *bcl = new QVBoxLayout(barCard);
    bcl->setContentsMargins(2, 2, 2, 2);
    barChart = new BarChart;
    barChart->setTitle(QS("Chiffre d'affaires par cat\u00e9gorie"));
    bcl->addWidget(barChart);

    QFrame *pieCard = new QFrame;
    pieCard->setObjectName("innerCard");
    QVBoxLayout *pcl = new QVBoxLayout(pieCard);
    pcl->setContentsMargins(2, 2, 2, 2);
    pieChart = new PieChart;
    pieChart->setTitle(QS("R\u00e9partition des clients par cat\u00e9gorie"));
    pcl->addWidget(pieChart);

    stl->addWidget(barCard, 1);
    stl->addWidget(pieCard, 1);

    QHBoxLayout *center = new QHBoxLayout;
    center->setSpacing(12);
    center->addWidget(tablePanel, 9);
    center->addWidget(statPanel, 5);
    root->addLayout(center, 5);

    // ---------- Bas : fidelite + ROI + formulaire ----------
    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->setSpacing(12);

    QFrame *fidelity = makePanel();
    QVBoxLayout *fl = new QVBoxLayout(fidelity);
    fl->setContentsMargins(16, 14, 16, 14);
    fl->setSpacing(12);
    fl->addWidget(makeTitle(QS("\U0001F4A1  Fid\u00e9lisation intelligente")));

    QFormLayout *ff = new QFormLayout;
    ff->setHorizontalSpacing(14);
    ff->setVerticalSpacing(14);
    ff->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    clientFideliteCombo = new QComboBox;
    ff->addRow(QS("Client :"), clientFideliteCombo);

    scoreBar = new QProgressBar;
    scoreBar->setRange(0, 100);
    scoreBar->setTextVisible(false);
    scoreBar->setFixedHeight(10);
    scoreLabel = new QLabel(QS("-"));
    scoreLabel->setObjectName("scoreText");
    QHBoxLayout *sr = new QHBoxLayout;
    sr->addWidget(scoreBar, 1);
    sr->addWidget(scoreLabel);
    ff->addRow(QS("Score de fid\u00e9lit\u00e9 :"), sr);

    remiseLabel = new QLabel(QS("-"));
    remiseLabel->setObjectName("pillGreen");
    remiseLabel->setAlignment(Qt::AlignCenter);
    remiseLabel->setMinimumWidth(90);
    remiseLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    ff->addRow(QS("Remise recommand\u00e9e :"), remiseLabel);

    fl->addLayout(ff);
    fl->addStretch();
    QPushButton *scoreButton = makeButton(QS("Calculer le score"), "primary");
    fl->addWidget(scoreButton, 0, Qt::AlignHCenter);

    QFrame *roi = makePanel();
    QVBoxLayout *rl = new QVBoxLayout(roi);
    rl->setContentsMargins(16, 14, 16, 14);
    rl->setSpacing(12);
    rl->addWidget(makeTitle(QS("\U0001F4CA  Calculateur de ROI & Analyse pr\u00e9dictive")));

    QFormLayout *rf = new QFormLayout;
    rf->setHorizontalSpacing(14);
    rf->setVerticalSpacing(12);
    rf->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    clientRoiCombo = new QComboBox;
    rf->addRow(QS("Client :"), clientRoiCombo);

    budgetSpin = new QDoubleSpinBox;
    budgetSpin->setRange(0, 10000000);
    budgetSpin->setDecimals(0);
    budgetSpin->setValue(5000);
    rf->addRow(QS("Budget investi (DT) :"), budgetSpin);

    hausseSpin = new QDoubleSpinBox;
    hausseSpin->setRange(0, 1000);
    hausseSpin->setDecimals(0);
    hausseSpin->setValue(18);
    rf->addRow(QS("Hausse des ventes (%) :"), hausseSpin);

    roiLabel = new QLabel(QS("-"));
    roiLabel->setObjectName("pillBlue");
    roiLabel->setAlignment(Qt::AlignCenter);
    rf->addRow(QS("ROI estim\u00e9 :"), roiLabel);

    rl->addLayout(rf);
    rl->addStretch();
    QPushButton *roiButton = makeButton(QS("Calculer le ROI"), "primary");
    rl->addWidget(roiButton, 0, Qt::AlignHCenter);

    bottom->addWidget(fidelity, 10);
    bottom->addWidget(roi, 10);
    bottom->addWidget(buildForm(), 13);
    root->addLayout(bottom, 3);

    // ---------- Connexions ----------
    connect(addButton, &QPushButton::clicked, this, [this]() { addItem(); });
    connect(modifyButton, &QPushButton::clicked, this, [this]() { modifyItem(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { removeItem(); });
    connect(exportButton, &QPushButton::clicked, this, [this]() { exportCsv(); });
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { refreshTable(); });
    connect(categorieFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(scoreButton, &QPushButton::clicked, this, [this]() { calculateFidelity(); });
    connect(clientFideliteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { calculateFidelity(); });
    connect(roiButton, &QPushButton::clicked, this, [this]() { calculateROI(); });
    connect(clientRoiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { calculateROI(); });
    connect(budgetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { calculateROI(); });
    connect(hausseSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { calculateROI(); });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this]()
            {
                const int id = rowId(table);
                if (id < 0)
                    return;

                status->cancel(true);

                if (const Client *c = find(id))
                    fillForm(*c);
            });
}

QWidget *ClientPage::buildForm()
{
    QFrame *panel = makePanel();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(8);

    QPushButton *clearButton = makeButton(QS("Effacer"), "secondary");
    clearButton->setStyleSheet(QS("padding:5px 14px;"));
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->addWidget(makeTitle(QS("\U0001F464  Ajouter / Modifier un client")));
    titleRow->addStretch();
    titleRow->addWidget(clearButton);
    layout->addLayout(titleRow);

    idEdit = new QLineEdit;
    idEdit->setValidator(new QIntValidator(1, 999999, idEdit));
    nomEdit = new QLineEdit;
    nomEdit->setPlaceholderText(QS("Ex : Aqua Plus"));
    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText(QS("contact@exemple.tn"));
    telephoneEdit = new QLineEdit;
    telephoneEdit->setPlaceholderText(QS("71 123 456"));

    categorieCombo = new QComboBox;
    categorieCombo->addItems({catGrossiste(), catDetaillant(), catEntreprise()});

    caSpin = new QDoubleSpinBox;
    caSpin->setRange(0, 100000000);
    caSpin->setDecimals(2);
    caSpin->setSuffix(QS(" DT"));

    QGridLayout *grid = makeGrid(2);
    addField(grid, 0, 0, QS("ID client"), idEdit);
    addField(grid, 0, 1, QS("Nom / Raison sociale"), nomEdit);
    addField(grid, 1, 0, QS("Email"), emailEdit);
    addField(grid, 1, 1, QS("T\u00e9l\u00e9phone"), telephoneEdit);
    addField(grid, 2, 0, QS("Cat\u00e9gorie"), categorieCombo);
    addField(grid, 2, 1, QS("Chiffre d'affaires"), caSpin);
    layout->addLayout(grid);
    layout->addStretch();

    status = new StatusBox(QS("Cliquez sur un client de la liste pour modifier ses informations, "
                              "ou remplissez le formulaire puis cliquez sur \u00ab Ajouter \u00bb."));
    layout->addWidget(status);

    connect(clearButton, &QPushButton::clicked, this, [this]()
            {
                status->reset();
                clearForm();
            });

    return panel;
}

void ClientPage::refreshTable()
{
    table->setRowCount(0);

    const QString search = searchEdit->text().trimmed().toLower();
    const bool allCats = categorieFilter->currentIndex() == 0;
    const QString category = categorieFilter->currentText();
    const int sortMode = triCombo->currentIndex();

    QVector<Client> list = clients;
    std::stable_sort(list.begin(), list.end(),
                     [sortMode](const Client &a, const Client &b)
                     {
                         if (sortMode == 1) return a.chiffreAffaires > b.chiffreAffaires;
                         if (sortMode == 2) return a.categorie < b.categorie;
                         return a.id < b.id;
                     });

    for (const Client &c : list)
    {
        const bool okSearch = search.isEmpty()
        || c.nom.toLower().contains(search)
            || c.email.toLower().contains(search)
            || c.telephone.toLower().contains(search);

        if (!okSearch || (!allCats && c.categorie != category))
            continue;

        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, idItem(code('C', c.id), c.id));
        table->setItem(row, 1, new QTableWidgetItem(c.nom));
        table->setItem(row, 2, new QTableWidgetItem(c.email));
        table->setItem(row, 3, new QTableWidgetItem(c.telephone));
        table->setItem(row, 4, new QTableWidgetItem(QString()));

        QTableWidgetItem *ca = new QTableWidgetItem(spaced(c.chiffreAffaires));
        ca->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 5, ca);

        QString bg = QS("#DDEBFF"), fg = QS("#1459A6");
        if (c.categorie == catGrossiste())       { bg = QS("#D9FBEA"); fg = QS("#138A5B"); }
        else if (c.categorie == catDetaillant()) { bg = QS("#FFE9D2"); fg = QS("#C45B00"); }
        setPill(table, row, 4, c.categorie, bg, fg);
    }
}

void ClientPage::updateStatistics()
{
    double totalCA = 0, caG = 0, caD = 0, caE = 0;
    int nG = 0, nD = 0, nE = 0;

    for (const Client &c : clients)
    {
        totalCA += c.chiffreAffaires;

        if (c.categorie == catGrossiste())       { ++nG; caG += c.chiffreAffaires; }
        else if (c.categorie == catDetaillant()) { ++nD; caD += c.chiffreAffaires; }
        else                                     { ++nE; caE += c.chiffreAffaires; }
    }

    nombreLabel->setText(QString::number(clients.size()));
    caTotalLabel->setText(spaced(totalCA) + QS(" DT"));
    grossistesLabel->setText(QString::number(nG));
    detaillantsLabel->setText(QString::number(nD));
    entreprisesLabel->setText(QString::number(nE));

    barChart->setData({caG, caD, caE}, {catGrossiste(), catDetaillant(), catEntreprise()});
    pieChart->setData({double(nG), double(nD), double(nE)},
                      {QS("Grossistes"), QS("D\u00e9taillants"), QS("Entreprises")});

    QVector<QPair<QString, int>> items;
    for (const Client &c : clients)
        items.append(qMakePair(c.nom, c.id));

    fillIdCombo(clientFideliteCombo, items);
    fillIdCombo(clientRoiCombo, items);

    calculateFidelity();
    calculateROI();
}

void ClientPage::clearForm()
{
    table->clearSelection();

    int next = 1;
    for (const Client &c : clients)
        next = qMax(next, c.id + 1);

    idEdit->setText(QString::number(next));
    nomEdit->clear();
    emailEdit->clear();
    telephoneEdit->clear();
    categorieCombo->setCurrentIndex(0);
    caSpin->setValue(0);
}

void ClientPage::fillForm(const Client &c)
{
    idEdit->setText(QString::number(c.id));
    nomEdit->setText(c.nom);
    emailEdit->setText(c.email);
    telephoneEdit->setText(c.telephone);

    const int index = categorieCombo->findText(c.categorie);
    categorieCombo->setCurrentIndex(index >= 0 ? index : 0);
    caSpin->setValue(c.chiffreAffaires);
}

bool ClientPage::readForm(Client &out, bool isNew)
{
    bool ok = false;
    const int id = idEdit->text().trimmed().toInt(&ok);

    if (!ok || id <= 0)
    {
        status->say(QS("Veuillez saisir un ID client valide."), StatusBox::Error);
        return false;
    }

    if (isNew && find(id))
    {
        status->say(QS("Cet ID client existe d\u00e9j\u00e0."), StatusBox::Error);
        return false;
    }

    const QString nom = nomEdit->text().trimmed();
    const QString email = emailEdit->text().trimmed();
    const QString tel = telephoneEdit->text().trimmed();

    if (nom.isEmpty())
    {
        status->say(QS("Le nom est obligatoire."), StatusBox::Error);
        return false;
    }

    if (!validEmail(email))
    {
        status->say(QS("Adresse email invalide."), StatusBox::Error);
        return false;
    }

    if (tel.isEmpty())
    {
        status->say(QS("Le t\u00e9l\u00e9phone est obligatoire."), StatusBox::Error);
        return false;
    }

    out.id = id;
    out.nom = nom;
    out.email = email;
    out.telephone = tel;
    out.categorie = categorieCombo->currentText();
    out.chiffreAffaires = caSpin->value();
    return true;
}

void ClientPage::addItem()
{
    status->reset();

    Client c{};
    if (!readForm(c, true))
        return;

    status->ask(QS("Ajouter \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(c.nom),
                [this, c]()
                {
                    clients.append(c);
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("Client ajout\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ClientPage::modifyItem()
{
    status->reset();

    const int id = rowId(table);
    if (id < 0)
    {
        status->say(QS("S\u00e9lectionnez d'abord un client dans la liste."), StatusBox::Error);
        return;
    }

    Client c{};
    if (!readForm(c, false))
        return;

    if (c.id != id)
    {
        status->say(QS("L'ID d'un client ne peut pas \u00eatre modifi\u00e9."), StatusBox::Error);
        return;
    }

    status->ask(QS("Enregistrer les modifications de \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(c.nom),
                [this, c, id]()
                {
                    for (Client &old : clients)
                        if (old.id == id)
                            old = c;

                    refreshTable();
                    updateStatistics();
                    selectById(table, id);
                    status->say(QS("Client modifi\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ClientPage::removeItem()
{
    status->reset();

    const int id = rowId(table);
    const Client *c = find(id);
    if (!c)
    {
        status->say(QS("S\u00e9lectionnez d'abord un client dans la liste."), StatusBox::Error);
        return;
    }

    status->ask(QS("Supprimer \u00ab %1 \u00bb ?   + confirmer  /  \u2212 annuler").arg(c->nom),
                [this, id]()
                {
                    clients.erase(std::remove_if(clients.begin(), clients.end(),
                                                 [id](const Client &x) { return x.id == id; }),
                                  clients.end());
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("Client supprim\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void ClientPage::exportCsv()
{
    QStringList lines;
    for (const Client &c : clients)
        lines << QS("%1;%2;%3;%4;%5;%6")
                     .arg(code('C', c.id), csvq(c.nom), csvq(c.email), csvq(c.telephone),
                          csvq(c.categorie), QString::number(c.chiffreAffaires, 'f', 2));

    exportTo(this, status, QS("clients"), QS("clients.csv"),
             QS("ID;Nom;Email;Telephone;Categorie;ChiffreAffaires"), lines);
}

void ClientPage::calculateFidelity()
{
    if (clientFideliteCombo->currentIndex() < 0)
    {
        scoreBar->setValue(0);
        scoreLabel->setText(QS("-"));
        remiseLabel->setText(QS("-"));
        return;
    }

    const Client *c = find(clientFideliteCombo->currentData().toInt());
    const double ca = c ? c->chiffreAffaires : 0;

    // 100 000 DT de CA = score 100
    const int score = qBound(0, static_cast<int>(std::lround(ca / 1000.0)), 100);

    int remise = 2;
    if (score >= 80)      remise = 10;
    else if (score >= 60) remise = 7;
    else if (score >= 40) remise = 5;

    scoreBar->setValue(score);
    scoreLabel->setText(QS("%1 / 100").arg(score));
    remiseLabel->setText(QS("%1 %").arg(remise));
}

void ClientPage::calculateROI()
{
    if (clientRoiCombo->currentIndex() < 0)
    {
        roiLabel->setText(QS("-"));
        return;
    }

    const double budget = budgetSpin->value();
    if (budget <= 0)
    {
        roiLabel->setText(QS("Budget > 0 requis"));
        return;
    }

    const Client *c = find(clientRoiCombo->currentData().toInt());
    const double ca = c ? c->chiffreAffaires : 0;

    const double gain = ca * (hausseSpin->value() / 100.0);
    const double roi = ((gain - budget) / budget) * 100.0;

    roiLabel->setText(QS("%1 %").arg(QString::number(roi, 'f', 1)));
}

// ============================================================
// MODULE EMPLOYES
// ============================================================

EmployePage::EmployePage(QWidget *parent)
    : Page(parent)
{
    load();
    build();
    refreshTable();
    updateStatistics();
    clearForm();
}

QString EmployePage::summary() const
{
    int absents = 0;
    for (const Employe &e : employes)
        if (e.statut != sPresent())
            ++absents;

    return QS("%1 employ\u00e9s \u2022 %2 absent(s) aujourd'hui").arg(employes.size()).arg(absents);
}

void EmployePage::load()
{
    employes.clear();
    const QDate t = QDate::currentDate();

    auto add = [this, t](int id, const QString &nom, const QString &prenom, int age,
                         const QString &email, const QString &tel, const QString &poste,
                         const QString &dep, const QDate &embauche, const QString &statut,
                         const QVector<int> &absDaysAgo)
    {
        QVector<QDate> abs;
        for (int d : absDaysAgo)
            abs.append(t.addDays(-d));

        employes.append(Employe{id, nom, prenom, age, email, tel, poste, dep, embauche, statut, abs});
    };

    const QString dev = QS("D\u00e9veloppeur");
    const QString chef = QS("Chef d'\u00e9quipe");
    const QString rh = QS("Responsable RH");

    add(1, QS("Ben Ali"), QS("Sami"), 32, QS("sami.benali@hydro.tn"), QS("71 123 456"), dev,
        QS("Informatique"), QDate(2019, 3, 12), sPresent(), {100, 260});
    add(2, QS("Trabelsi"), QS("Ines"), 28, QS("ines.trabelsi@hydro.tn"), QS("98 765 432"), QS("Comptable"),
        QS("Comptabilit\u00e9"), QDate(2021, 9, 1), sConge(), {0, 20, 35});
    add(3, QS("Mansouri"), QS("Karim"), 45, QS("karim.mansouri@hydro.tn"), QS("22 345 678"), chef,
        QS("Production"), QDate(2015, 1, 20), sAbsent(), {0, 10, 40, 75, 120, 200, 300});
    add(4, QS("Gharbi"), QS("Leila"), 35, QS("leila.gharbi@hydro.tn"), QS("55 987 654"), rh,
        QS("Ressources Humaines"), QDate(2018, 6, 4), sPresent(), {45, 190});
    add(5, QS("Khemiri"), QS("Yassine"), 26, QS("yassine.khemiri@hydro.tn"), QS("23 456 789"), QS("Commercial"),
        QS("Ventes"), QDate(2022, 2, 14), sPresent(), {});
    add(6, QS("Bouazizi"), QS("Amira"), 30, QS("amira.bouazizi@hydro.tn"), QS("96 321 654"), dev,
        QS("Informatique"), QDate(2020, 11, 8), sPresent(), {70});
    add(7, QS("Jelassi"), QS("Mehdi"), 40, QS("mehdi.jelassi@hydro.tn"), QS("27 654 321"), QS("Ouvrier"),
        QS("Production"), QDate(2016, 7, 22), sAbsent(), {0, 5, 60, 150, 220});
    add(8, QS("Sassi"), QS("Nour"), 24, QS("nour.sassi@hydro.tn"), QS("52 963 741"), QS("Commercial"),
        QS("Ventes"), QDate(2023, 4, 3), sPresent(), {});
}

const Employe *EmployePage::find(int id) const
{
    for (const Employe &e : employes)
        if (e.id == id)
            return &e;
    return nullptr;
}

void EmployePage::build()
{
    const QStringList deps = {QS("Ressources Humaines"), QS("Informatique"), QS("Ventes"),
                              QS("Comptabilit\u00e9"), QS("Production")};

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(22, 16, 22, 16);
    root->setSpacing(12);

    root->addWidget(makeHeader(QS("\U0001F477  Gestion des Employ\u00e9s"),
                               QS("Ajoutez, modifiez et suivez les employ\u00e9s de l'usine HydroConnect")));

    QHBoxLayout *cards = new QHBoxLayout;
    cards->setSpacing(12);
    cards->addWidget(makeCard(QS("Total employ\u00e9s"), QS("\U0001F465"), QS("#E3F0FF"), &totalLabel));
    cards->addWidget(makeCard(QS("Pr\u00e9sents"), QS("\u2705"), QS("#D9FBEA"), &presentsLabel));
    cards->addWidget(makeCard(QS("Absents"), QS("\u26A0"), QS("#FDE8E8"), &absentsLabel));
    cards->addWidget(makeCard(QS("En cong\u00e9"), QS("\U0001F3D6"), QS("#FFF0E0"), &congeLabel));
    cards->addWidget(makeCard(QS("Taux de pr\u00e9sence"), QS("\U0001F4C8"), QS("#EFE8FF"), &tauxLabel));
    root->addLayout(cards);

    // ---------- Tableau ----------
    QFrame *tablePanel = makePanel();
    QVBoxLayout *tl = new QVBoxLayout(tablePanel);
    tl->setContentsMargins(14, 12, 14, 12);
    tl->setSpacing(10);
    tl->addWidget(makeTitle(QS("\U0001F465  Liste des employ\u00e9s")));

    searchEdit = new QLineEdit;
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setPlaceholderText(QS("\U0001F50E  Rechercher (ID, nom, pr\u00e9nom, email, poste, d\u00e9partement)..."));

    triCombo = new QComboBox;
    triCombo->addItems({QS("Aucun"), QS("Nom"), QS("\u00c2ge"), QS("Poste"), QS("D\u00e9partement")});
    triCombo->setMinimumWidth(130);

    depFilter = new QComboBox;
    depFilter->addItem(QS("Tous"));
    depFilter->addItems(deps);
    depFilter->setMinimumWidth(150);

    QHBoxLayout *sl = new QHBoxLayout;
    sl->setSpacing(8);
    sl->addWidget(searchEdit, 1);
    sl->addWidget(new QLabel(QS("Trier par :")));
    sl->addWidget(triCombo);
    sl->addWidget(new QLabel(QS("D\u00e9partement :")));
    sl->addWidget(depFilter);
    tl->addLayout(sl);

    table = makeTable({QS("ID"), QS("Nom"), QS("Pr\u00e9nom"), QS("\u00c2ge"), QS("Email"), QS("T\u00e9l\u00e9phone"),
                       QS("Poste"), QS("D\u00e9partement"), QS("Embauche"), QS("Statut")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Fixed);
    table->setColumnWidth(9, 110);
    tl->addWidget(table, 1);

    QPushButton *addButton = makeButton(QS("\uff0b  Ajouter"), "primary");
    QPushButton *modifyButton = makeButton(QS("\u270e  Modifier"), "warning");
    QPushButton *deleteButton = makeButton(QS("\U0001F5D1  Supprimer"), "danger");
    QPushButton *exportButton = makeButton(QS("\U0001F4C4  Exporter Excel"), "secondary");

    QHBoxLayout *bl = new QHBoxLayout;
    bl->setSpacing(10);
    bl->addWidget(addButton);
    bl->addWidget(modifyButton);
    bl->addWidget(deleteButton);
    bl->addWidget(exportButton);
    bl->addStretch();
    tl->addLayout(bl);

    // ---------- Statistiques ----------
    QFrame *statPanel = makePanel();
    QVBoxLayout *stl = new QVBoxLayout(statPanel);
    stl->setContentsMargins(14, 12, 14, 12);
    stl->setSpacing(10);
    stl->addWidget(makeTitle(QS("\U0001F4CA  Statistiques employ\u00e9s")));

    QFrame *barCard = new QFrame;
    barCard->setObjectName("innerCard");
    QVBoxLayout *bcl = new QVBoxLayout(barCard);
    bcl->setContentsMargins(2, 2, 2, 2);
    barChart = new BarChart;
    barChart->setTitle(QS("Fr\u00e9quence des absences (6 derniers mois)"));
    barChart->setColors({QColor("#EF4444")});
    bcl->addWidget(barChart);

    QFrame *pieCard = new QFrame;
    pieCard->setObjectName("innerCard");
    QVBoxLayout *pcl = new QVBoxLayout(pieCard);
    pcl->setContentsMargins(2, 2, 2, 2);
    pieChart = new PieChart;
    pieChart->setTitle(QS("R\u00e9partition des employ\u00e9s par d\u00e9partement"));
    pieChart->setColors({QColor("#20B486"), QColor("#2678E8"), QColor("#EF4444"),
                         QColor("#9B59B6"), QColor("#F7C948")});
    pcl->addWidget(pieChart);

    stl->addWidget(barCard, 1);
    stl->addWidget(pieCard, 1);

    QHBoxLayout *center = new QHBoxLayout;
    center->setSpacing(12);
    center->addWidget(tablePanel, 9);
    center->addWidget(statPanel, 5);
    root->addLayout(center, 5);

    // ---------- Bas : analyse IA + absences du jour + formulaire ----------
    QHBoxLayout *bottom = new QHBoxLayout;
    bottom->setSpacing(12);

    QFrame *ia = makePanel();
    QVBoxLayout *il = new QVBoxLayout(ia);
    il->setContentsMargins(14, 12, 14, 12);
    il->setSpacing(10);
    il->addWidget(makeTitle(QS("\U0001F9E0  Analyse IA des absences")));

    QFormLayout *form = new QFormLayout;
    form->setHorizontalSpacing(10);
    form->setVerticalSpacing(12);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    analyseCombo = new QComboBox;
    form->addRow(QS("Employ\u00e9 :"), analyseCombo);

    riskBar = new QProgressBar;
    riskBar->setRange(0, 100);
    riskBar->setTextVisible(false);
    riskBar->setFixedHeight(10);
    riskLabel = new QLabel(QS("-"));
    riskLabel->setObjectName("scoreText");
    QHBoxLayout *rr = new QHBoxLayout;
    rr->addWidget(riskBar, 1);
    rr->addWidget(riskLabel);
    form->addRow(QS("Score de risque :"), rr);

    riskLevelLabel = new QLabel(QS("-"));
    riskLevelLabel->setAlignment(Qt::AlignCenter);
    riskLevelLabel->setMinimumWidth(90);
    riskLevelLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    form->addRow(QS("Niveau de risque :"), riskLevelLabel);

    entretienLabel = new QLabel(QS("-"));
    entretienLabel->setObjectName("pillBlue");
    entretienLabel->setAlignment(Qt::AlignCenter);
    form->addRow(QS("Entretien conseill\u00e9 :"), entretienLabel);
    il->addLayout(form);

    analyseInfo = new QLabel;
    analyseInfo->setWordWrap(true);
    analyseInfo->setStyleSheet(QS("color:#526173; font-size:12px;"));
    il->addWidget(analyseInfo);
    il->addStretch();

    QPushButton *iaButton = makeButton(QS("Analyser l'employ\u00e9"), "primary");
    il->addWidget(iaButton, 0, Qt::AlignHCenter);

    alertPanel = new QFrame;
    alertPanel->setObjectName("alertPanel");
    QVBoxLayout *al = new QVBoxLayout(alertPanel);
    al->setContentsMargins(14, 12, 14, 12);
    al->setSpacing(8);

    alertBadge = new QLabel;
    alertBadge->setAlignment(Qt::AlignCenter);
    QPushButton *verifier = makeButton(QS("V\u00e9rifier"), "secondary");
    verifier->setStyleSheet(QS("padding:5px 14px;"));

    QHBoxLayout *ah = new QHBoxLayout;
    ah->addWidget(makeTitle(QS("\U0001F4C5  Absences du jour")));
    ah->addStretch();
    ah->addWidget(alertBadge);
    ah->addWidget(verifier);
    al->addLayout(ah);

    alertList = new QListWidget;
    alertList->setSelectionMode(QAbstractItemView::NoSelection);
    alertList->setWordWrap(true);
    al->addWidget(alertList, 1);

    bottom->addWidget(ia, 10);
    bottom->addWidget(alertPanel, 9);
    bottom->addWidget(buildForm(), 14);
    root->addLayout(bottom, 3);

    // ---------- Connexions ----------
    connect(addButton, &QPushButton::clicked, this, [this]() { addItem(); });
    connect(modifyButton, &QPushButton::clicked, this, [this]() { modifyItem(); });
    connect(deleteButton, &QPushButton::clicked, this, [this]() { removeItem(); });
    connect(exportButton, &QPushButton::clicked, this, [this]() { exportCsv(); });
    connect(searchEdit, &QLineEdit::textChanged, this, [this](const QString &) { refreshTable(); });
    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(depFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { refreshTable(); });
    connect(iaButton, &QPushButton::clicked, this, [this]() { runAnalysis(); });
    connect(analyseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { runAnalysis(); });
    connect(verifier, &QPushButton::clicked, this, [this]() { refreshAlerts(); });
    connect(table, &QTableWidget::itemSelectionChanged, this, [this]()
            {
                const int id = rowId(table);
                if (id < 0)
                    return;

                status->cancel(true);

                if (const Employe *e = find(id))
                    fillForm(*e);
            });
}

QWidget *EmployePage::buildForm()
{
    QFrame *panel = makePanel();
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    QPushButton *clearButton = makeButton(QS("Effacer"), "secondary");
    clearButton->setStyleSheet(QS("padding:5px 14px;"));
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->addWidget(makeTitle(QS("\U0001F464  Ajouter / Modifier un employ\u00e9")));
    titleRow->addStretch();
    titleRow->addWidget(clearButton);
    layout->addLayout(titleRow);

    idEdit = new QLineEdit;
    idEdit->setValidator(new QIntValidator(1, 999999, idEdit));
    nomEdit = new QLineEdit;
    prenomEdit = new QLineEdit;

    ageSpin = new QSpinBox;
    ageSpin->setRange(16, 80);
    ageSpin->setValue(25);

    emailEdit = new QLineEdit;
    emailEdit->setPlaceholderText(QS("nom@hydro.tn"));
    telEdit = new QLineEdit;
    telEdit->setPlaceholderText(QS("71 123 456"));

    posteCombo = new QComboBox;
    posteCombo->addItems({QS("D\u00e9veloppeur"), QS("Chef d'\u00e9quipe"), QS("Commercial"), QS("Comptable"),
                          QS("Responsable RH"), QS("Ouvrier")});

    depCombo = new QComboBox;
    depCombo->addItems({QS("Ressources Humaines"), QS("Informatique"), QS("Ventes"),
                        QS("Comptabilit\u00e9"), QS("Production")});

    embaucheEdit = makeDate(QDate::currentDate());

    statutCombo = new QComboBox;
    statutCombo->addItems({sPresent(), sAbsent(), sConge()});

    QGridLayout *grid = makeGrid(3);
    addField(grid, 0, 0, QS("ID employ\u00e9"), idEdit);
    addField(grid, 0, 1, QS("Nom"), nomEdit);
    addField(grid, 0, 2, QS("Pr\u00e9nom"), prenomEdit);
    addField(grid, 1, 0, QS("\u00c2ge"), ageSpin);
    addField(grid, 1, 1, QS("Email"), emailEdit);
    addField(grid, 1, 2, QS("T\u00e9l\u00e9phone"), telEdit);
    addField(grid, 2, 0, QS("Poste"), posteCombo);
    addField(grid, 2, 1, QS("D\u00e9partement"), depCombo);
    addField(grid, 2, 2, QS("Date d'embauche"), embaucheEdit);
    addField(grid, 3, 0, QS("Statut du jour"), statutCombo);
    layout->addLayout(grid);
    layout->addStretch();

    status = new StatusBox(QS("Cliquez sur un employ\u00e9 de la liste pour le modifier, "
                              "ou remplissez le formulaire puis cliquez sur \u00ab Ajouter \u00bb."));
    layout->addWidget(status);

    connect(clearButton, &QPushButton::clicked, this, [this]()
            {
                status->reset();
                clearForm();
            });

    return panel;
}

void EmployePage::refreshTable()
{
    table->setRowCount(0);

    const QString search = searchEdit->text().trimmed().toLower();
    const bool allDeps = depFilter->currentIndex() == 0;
    const QString dep = depFilter->currentText();
    const int sortMode = triCombo->currentIndex();

    QVector<Employe> list = employes;
    std::stable_sort(list.begin(), list.end(),
                     [sortMode](const Employe &a, const Employe &b)
                     {
                         switch (sortMode)
                         {
                         case 1: if (a.nom != b.nom) return a.nom < b.nom; break;
                         case 2: if (a.age != b.age) return a.age < b.age; break;
                         case 3: if (a.poste != b.poste) return a.poste < b.poste; break;
                         case 4: if (a.departement != b.departement) return a.departement < b.departement; break;
                         default: break;
                         }
                         return a.id < b.id;
                     });

    for (const Employe &e : list)
    {
        const bool okSearch = search.isEmpty()
        || code('E', e.id).toLower().contains(search)
            || e.nom.toLower().contains(search)
            || e.prenom.toLower().contains(search)
            || e.email.toLower().contains(search)
            || e.poste.toLower().contains(search)
            || e.departement.toLower().contains(search);

        if (!okSearch || (!allDeps && e.departement != dep))
            continue;

        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, idItem(code('E', e.id), e.id));
        table->setItem(row, 1, new QTableWidgetItem(e.nom));
        table->setItem(row, 2, new QTableWidgetItem(e.prenom));
        table->setItem(row, 3, new QTableWidgetItem(QString::number(e.age)));
        table->setItem(row, 4, new QTableWidgetItem(e.email));
        table->setItem(row, 5, new QTableWidgetItem(e.telephone));
        table->setItem(row, 6, new QTableWidgetItem(e.poste));
        table->setItem(row, 7, new QTableWidgetItem(e.departement));
        table->setItem(row, 8, new QTableWidgetItem(fmtDate(e.dateEmbauche)));
        table->setItem(row, 9, new QTableWidgetItem(QString()));

        QString bg, fg;
        if (e.statut == sPresent())     { bg = QS("#D9FBEA"); fg = QS("#138A5B"); }
        else if (e.statut == sAbsent()) { bg = QS("#FDE8E8"); fg = QS("#B42318"); }
        else                            { bg = QS("#FFE9D2"); fg = QS("#C45B00"); }
        setPill(table, row, 9, e.statut, bg, fg);
    }
}

void EmployePage::updateStatistics()
{
    int present = 0, absent = 0, conge = 0;
    QVector<double> depCount(5, 0.0);
    const QStringList deps = {QS("Ressources Humaines"), QS("Informatique"), QS("Ventes"),
                              QS("Comptabilit\u00e9"), QS("Production")};

    for (const Employe &e : employes)
    {
        if (e.statut == sPresent())     ++present;
        else if (e.statut == sAbsent()) ++absent;
        else                            ++conge;

        const int i = deps.indexOf(e.departement);
        if (i >= 0)
            depCount[i] += 1.0;
    }

    const int total = employes.size();
    const double taux = total > 0 ? present * 100.0 / total : 0.0;

    totalLabel->setText(QString::number(total));
    presentsLabel->setText(QString::number(present));
    absentsLabel->setText(QString::number(absent));
    congeLabel->setText(QString::number(conge));
    tauxLabel->setText(QS("%1 %").arg(QString::number(taux, 'f', 1)));

    const QDate today = QDate::currentDate();
    const QLocale fr(QLocale::French);

    QVector<double> counts(6, 0.0);
    QVector<QString> labels;

    for (int i = 0; i < 6; ++i)
    {
        const QDate month = today.addMonths(-(5 - i));
        labels.append(fr.monthName(month.month(), QLocale::ShortFormat));

        for (const Employe &e : employes)
            for (const QDate &d : e.absences)
                if (d.year() == month.year() && d.month() == month.month())
                    counts[i] += 1.0;
    }

    barChart->setData(counts, labels);
    pieChart->setData(depCount, {QS("RH"), QS("Informatique"), QS("Ventes"),
                                 QS("Comptabilit\u00e9"), QS("Production")});

    QVector<QPair<QString, int>> items;
    for (const Employe &e : employes)
        items.append(qMakePair(code('E', e.id) + QS(" - ") + e.nom + QS(" ") + e.prenom, e.id));

    fillIdCombo(analyseCombo, items);
    runAnalysis();
    refreshAlerts();
}

void EmployePage::clearForm()
{
    table->clearSelection();

    int next = 1;
    for (const Employe &e : employes)
        next = qMax(next, e.id + 1);

    idEdit->setText(QString::number(next));
    nomEdit->clear();
    prenomEdit->clear();
    ageSpin->setValue(25);
    emailEdit->clear();
    telEdit->clear();
    posteCombo->setCurrentIndex(0);
    depCombo->setCurrentIndex(0);
    embaucheEdit->setDate(QDate::currentDate());
    statutCombo->setCurrentIndex(0);
}

void EmployePage::fillForm(const Employe &e)
{
    idEdit->setText(QString::number(e.id));
    nomEdit->setText(e.nom);
    prenomEdit->setText(e.prenom);
    ageSpin->setValue(e.age);
    emailEdit->setText(e.email);
    telEdit->setText(e.telephone);

    int i = posteCombo->findText(e.poste);
    posteCombo->setCurrentIndex(i >= 0 ? i : 0);
    i = depCombo->findText(e.departement);
    depCombo->setCurrentIndex(i >= 0 ? i : 0);
    i = statutCombo->findText(e.statut);
    statutCombo->setCurrentIndex(i >= 0 ? i : 0);

    embaucheEdit->setDate(e.dateEmbauche);
}

bool EmployePage::readForm(Employe &out, bool isNew)
{
    bool ok = false;
    const int id = idEdit->text().trimmed().toInt(&ok);

    if (!ok || id <= 0)
    {
        status->say(QS("Veuillez saisir un ID employ\u00e9 valide."), StatusBox::Error);
        return false;
    }

    if (isNew && find(id))
    {
        status->say(QS("Cet ID employ\u00e9 existe d\u00e9j\u00e0."), StatusBox::Error);
        return false;
    }

    const QString nom = nomEdit->text().trimmed();
    const QString prenom = prenomEdit->text().trimmed();
    const QString email = emailEdit->text().trimmed();
    const QString tel = telEdit->text().trimmed();

    if (nom.isEmpty() || prenom.isEmpty())
    {
        status->say(QS("Le nom et le pr\u00e9nom sont obligatoires."), StatusBox::Error);
        return false;
    }

    if (!validEmail(email))
    {
        status->say(QS("Adresse email invalide."), StatusBox::Error);
        return false;
    }

    if (tel.isEmpty())
    {
        status->say(QS("Le t\u00e9l\u00e9phone est obligatoire."), StatusBox::Error);
        return false;
    }

    if (embaucheEdit->date() > QDate::currentDate())
    {
        status->say(QS("La date d'embauche ne peut pas \u00eatre dans le futur."), StatusBox::Error);
        return false;
    }

    out.id = id;
    out.nom = nom;
    out.prenom = prenom;
    out.age = ageSpin->value();
    out.email = email;
    out.telephone = tel;
    out.poste = posteCombo->currentText();
    out.departement = depCombo->currentText();
    out.dateEmbauche = embaucheEdit->date();
    out.statut = statutCombo->currentText();
    return true;
}

void EmployePage::addItem()
{
    status->reset();

    Employe e{};
    if (!readForm(e, true))
        return;

    if (e.statut == sAbsent())
        e.absences.append(QDate::currentDate());

    status->ask(QS("Ajouter \u00ab %1 %2 \u00bb ?   + confirmer  /  \u2212 annuler").arg(e.nom, e.prenom),
                [this, e]()
                {
                    employes.append(e);
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("Employ\u00e9 ajout\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EmployePage::modifyItem()
{
    status->reset();

    const int id = rowId(table);
    if (id < 0)
    {
        status->say(QS("S\u00e9lectionnez d'abord un employ\u00e9 dans la liste."), StatusBox::Error);
        return;
    }

    Employe e{};
    if (!readForm(e, false))
        return;

    if (e.id != id)
    {
        status->say(QS("L'ID d'un employ\u00e9 ne peut pas \u00eatre modifi\u00e9."), StatusBox::Error);
        return;
    }

    if (const Employe *old = find(id))
    {
        e.absences = old->absences;
        if (old->statut != sAbsent() && e.statut == sAbsent())
            e.absences.append(QDate::currentDate());
    }

    status->ask(QS("Enregistrer les modifications de \u00ab %1 %2 \u00bb ?   + confirmer  /  \u2212 annuler")
                    .arg(e.nom, e.prenom),
                [this, e, id]()
                {
                    for (Employe &old : employes)
                        if (old.id == id)
                            old = e;

                    refreshTable();
                    updateStatistics();
                    selectById(table, id);
                    status->say(QS("Employ\u00e9 modifi\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EmployePage::removeItem()
{
    status->reset();

    const int id = rowId(table);
    const Employe *e = find(id);
    if (!e)
    {
        status->say(QS("S\u00e9lectionnez d'abord un employ\u00e9 dans la liste."), StatusBox::Error);
        return;
    }

    status->ask(QS("Supprimer \u00ab %1 %2 \u00bb ?   + confirmer  /  \u2212 annuler").arg(e->nom, e->prenom),
                [this, id]()
                {
                    employes.erase(std::remove_if(employes.begin(), employes.end(),
                                                  [id](const Employe &x) { return x.id == id; }),
                                   employes.end());
                    refreshTable();
                    updateStatistics();
                    clearForm();
                    status->say(QS("Employ\u00e9 supprim\u00e9 avec succ\u00e8s."), StatusBox::Success);
                });
}

void EmployePage::exportCsv()
{
    QStringList lines;
    for (const Employe &e : employes)
        lines << QS("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11")
                     .arg(code('E', e.id), csvq(e.nom), csvq(e.prenom), QString::number(e.age),
                          csvq(e.email), csvq(e.telephone), csvq(e.poste), csvq(e.departement),
                          fmtDate(e.dateEmbauche), csvq(e.statut))
                     .arg(e.absences.size());

    exportTo(this, status, QS("employ\u00e9s"), QS("employes.csv"),
             QS("ID;Nom;Prenom;Age;Email;Telephone;Poste;Departement;DateEmbauche;Statut;NbAbsences"),
             lines);
}

RiskResult EmployePage::analyse(const Employe &e) const
{
    const QDate today = QDate::currentDate();
    RiskResult r;
    int recent = 0;

    for (const QDate &d : e.absences)
    {
        const int age = d.daysTo(today);
        if (age >= 0 && age <= 365)
            ++r.count12m;
        if (age >= 0 && age <= 90)
            ++recent;
    }

    double score = qMin(r.count12m * 10.0, 60.0) + qMin(recent * 8.0, 24.0);
    if (e.statut == sAbsent())     score += 16;
    else if (e.statut == sConge()) score += 4;

    r.score = qBound(0, qRound(score), 100);

    if (r.score >= 70)      r.niveau = QS("\u00c9lev\u00e9");
    else if (r.score >= 40) r.niveau = QS("Moyen");
    else                    r.niveau = QS("Faible");

    const int days = r.score >= 70 ? 7 : qBound(14, 120 - r.score, 120);
    r.dateProposee = today.addDays(days);
    return r;
}

void EmployePage::runAnalysis()
{
    if (analyseCombo->currentIndex() < 0)
    {
        riskBar->setValue(0);
        riskLabel->setText(QS("-"));
        riskLevelLabel->setText(QS("-"));
        entretienLabel->setText(QS("-"));
        analyseInfo->clear();
        return;
    }

    const Employe *e = find(analyseCombo->currentData().toInt());
    if (!e)
        return;

    const RiskResult r = analyse(*e);

    applyRisk(riskBar, riskLevelLabel, r.score);
    riskLabel->setText(QS("%1 / 100").arg(r.score));
    riskLevelLabel->setText(r.niveau);
    entretienLabel->setText(fmtDate(r.dateProposee));

    // ~250 jours ouvres par an
    const int presence = qBound(0, 100 - r.count12m * 100 / 250, 100);

    analyseInfo->setText(QS("%1 absence(s) sur 12 mois  \u2022  taux de pr\u00e9sence %2 %  \u2022  statut : %3")
                             .arg(r.count12m).arg(presence).arg(e->statut));
}

void EmployePage::refreshAlerts()
{
    QVector<AlertItem> alerts;

    for (const Employe &e : employes)
    {
        const QString name = code('E', e.id) + QS(" - ") + e.nom + QS(" ") + e.prenom;

        if (e.statut == sAbsent())
            alerts.append({0, QS("\U0001F534  %1 : absent aujourd'hui").arg(name)});
        else if (e.statut == sConge())
            alerts.append({1, QS("\U0001F7E0  %1 : en cong\u00e9").arg(name)});

        const RiskResult r = analyse(e);
        if (r.score >= 70)
            alerts.append({2, QS("\u26A0  %1 : risque d'absent\u00e9isme \u00e9lev\u00e9 (%2/100)")
                                  .arg(name).arg(r.score)});
    }

    std::stable_sort(alerts.begin(), alerts.end(),
                     [](const AlertItem &a, const AlertItem &b) { return a.level < b.level; });

    alertList->clear();

    for (const AlertItem &a : alerts)
    {
        QListWidgetItem *item = new QListWidgetItem(a.text);
        item->setForeground(alertColor(a.level));
        alertList->addItem(item);
    }

    if (alerts.isEmpty())
    {
        alertList->addItem(QS("\u2705  Aucune absence aujourd'hui."));
        alertBadge->setText(QS("Aucune alerte"));
        alertBadge->setStyleSheet(QS("background:#D9FBEA; color:#138A5B; border-radius:6px;"
                                     "padding:5px 12px; font-weight:bold; font-size:12px;"));
        alertPanel->setStyleSheet(QS("QFrame#alertPanel { background:white;"
                                     "border:1px solid #DFE6EF; border-radius:10px; }"));
    }
    else
    {
        alertBadge->setText(QS("%1 alerte(s)").arg(alerts.size()));
        alertBadge->setStyleSheet(QS("background:#EF4444; color:white; border-radius:6px;"
                                     "padding:5px 12px; font-weight:bold; font-size:12px;"));
        alertPanel->setStyleSheet(QS("QFrame#alertPanel { background:white;"
                                     "border:2px solid #EF4444; border-radius:10px; }"));
    }
}

// ============================================================
// STYLE GLOBAL
// ============================================================

static QString appStyle()
{
    return QS(R"(
        QMainWindow { background: #F4F7FB; }

        QWidget {
            font-family: "Segoe UI", Arial;
            color: #172B4D;
        }

        /* ---------- Sidebar ---------- */
        QFrame#sidebar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #082B49, stop:1 #0B4F82);
        }
        QFrame#sidebar QLabel { color: #B8D5EA; background: transparent; }

        /* ---------- Titres ---------- */
        QLabel#pageTitle    { font-size: 28px; font-weight: bold; }
        QLabel#pageSubtitle { color: #526173; font-size: 14px; }
        QLabel#panelTitle   { font-size: 16px; font-weight: bold; }
        QLabel#smallLabel   { color: #526173; font-size: 12px; }
        QLabel#valueLabel   { font-size: 13px; font-weight: bold; }
        QLabel#percent      { color: #1459A6; font-size: 12px; font-weight: bold; }
        QLabel#scoreText    { font-size: 13px; font-weight: bold; }
        QLabel#homeTitle    { font-size: 30px; font-weight: bold; }
        QLabel#homeSub      { font-size: 14px; color: #526173; }

        /* ---------- Panneaux ---------- */
        QFrame#panel {
            background: white;
            border: 1px solid #DFE6EF;
            border-radius: 10px;
        }
        QFrame#alertPanel {
            background: white;
            border: 1px solid #DFE6EF;
            border-radius: 10px;
        }
        QFrame#innerCard {
            background: white;
            border: 1px solid #E3E9F1;
            border-radius: 8px;
        }

        /* ---------- Champs ---------- */
        QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {
            background: white;
            border: 1px solid #CBD5E1;
            border-radius: 6px;
            padding: 7px 9px;
            font-size: 13px;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus,
        QDoubleSpinBox:focus, QDateEdit:focus {
            border: 2px solid #2678E8;
        }
        QComboBox::drop-down, QDateEdit::drop-down { border: none; width: 26px; }
        QComboBox::down-arrow, QDateEdit::down-arrow {
            image: none;
            width: 0px;
            height: 0px;
            margin-right: 8px;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #526173;
        }
        QComboBox QAbstractItemView {
            background: white;
            selection-background-color: #DCEBFF;
            selection-color: #172B4D;
            border: 1px solid #CBD5E1;
        }

        /* ---------- Boutons ---------- */
        QPushButton {
            border-radius: 7px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton#primary { background: #2678E8; color: white; }
        QPushButton#primary:hover { background: #1668D5; }
        QPushButton#warning { background: #F7C948; color: #3A2A00; }
        QPushButton#warning:hover { background: #EDB92E; }
        QPushButton#danger { background: #EF4444; color: white; }
        QPushButton#danger:hover { background: #DC2626; }
        QPushButton#secondary {
            background: white;
            color: #172B4D;
            border: 1px solid #CBD5E1;
        }
        QPushButton#secondary:hover { background: #EEF3F8; }
        QPushButton#confirmBtn, QPushButton#cancelBtn {
            color: white;
            font-size: 20px;
            padding: 0px;
        }
        QPushButton#confirmBtn { background: #20B486; }
        QPushButton#confirmBtn:hover { background: #159B72; }
        QPushButton#cancelBtn { background: #EF4444; }
        QPushButton#cancelBtn:hover { background: #DC2626; }

        /* ---------- Navigation ---------- */
        QPushButton#nav {
            color: white;
            background: transparent;
            text-align: left;
            padding: 12px 12px;
            font-weight: normal;
        }
        QPushButton#nav:hover { background: #123E61; }
        QPushButton#nav:checked { background: #2678E8; font-weight: bold; }

        QPushButton#homeCard {
            background: white;
            border: 1px solid #DFE6EF;
            border-radius: 12px;
            padding: 24px;
            font-size: 17px;
        }
        QPushButton#homeCard:hover { background: #EAF2FF; border-color: #2678E8; }

        /* ---------- Tableaux et listes ---------- */
        QTableWidget {
            background: white;
            border: 1px solid #E3E9F1;
            gridline-color: #E5EAF0;
            selection-background-color: #DCEBFF;
            selection-color: #172B4D;
            font-size: 12px;
        }
        QTableWidget::item {
            padding: 6px;
            border-bottom: 1px solid #E9EEF5;
        }
        QHeaderView::section {
            background: #F3F7FC;
            padding: 10px;
            border: none;
            border-bottom: 1px solid #DCE4EE;
            font-weight: bold;
        }
        QListWidget {
            background: white;
            border: 1px solid #E3E9F1;
            border-radius: 8px;
            font-size: 12px;
        }
        QListWidget::item {
            padding: 6px;
            border-bottom: 1px solid #E9EEF5;
        }
        QScrollBar:vertical { background: #F1F5F9; width: 9px; }
        QScrollBar::handle:vertical { background: #B8C5D4; border-radius: 4px; }

        /* ---------- Barres de progression et pastilles ---------- */
        QProgressBar { background: #E5EAF0; border: none; border-radius: 5px; }
        QProgressBar::chunk { background: #20B486; border-radius: 5px; }
        QLabel#pillGreen {
            background: #D9FBEA;
            color: #138A5B;
            border-radius: 6px;
            padding: 8px 14px;
            font-size: 15px;
            font-weight: bold;
        }
        QLabel#pillBlue {
            background: #D6EAFF;
            color: #1459A6;
            border-radius: 6px;
            padding: 8px 14px;
            font-size: 16px;
            font-weight: bold;
        }
    )");
}

// ============================================================
// FENETRE PRINCIPALE
// ============================================================

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QS("HydroConnect - Smart Water Factory"));
    setWindowIcon(QIcon(loadLogo()));
    resize(1750, 950);
    setStyleSheet(appStyle());

    QWidget *central = new QWidget;
    setCentralWidget(central);

    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    pages = {new ProduitPage, new EquipementPage, new ClientPage, new EmployePage};
    homeTitles = {QS("\U0001F4E6  Gestion des produits"),
                  QS("\U0001F6E0  Gestion des \u00e9quipements"),
                  QS("\U0001F465  Gestion des clients"),
                  QS("\U0001F477  Gestion des employ\u00e9s")};

    root->addWidget(createSidebar());

    stack = new QStackedWidget;
    stack->addWidget(createHomePage());
    for (Page *p : pages)
        stack->addWidget(p);
    root->addWidget(stack, 1);

    goTo(0);
}

QWidget *AppWindow::createSidebar()
{
    QFrame *sidebar = new QFrame;
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(235);

    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(14, 20, 14, 12);
    layout->setSpacing(6);

    QLabel *logo = new QLabel;
    logo->setAlignment(Qt::AlignCenter);

    const QPixmap pix = loadLogo();
    if (!pix.isNull())
    {
        const qreal dpr = devicePixelRatioF();
        QPixmap scaled = pix.scaledToWidth(qRound(190 * dpr), Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        logo->setPixmap(scaled);
    }
    else
    {
        logo->setText(QS("HydroConnect"));
        logo->setStyleSheet(QS("color:white; font-size:18px; font-weight:bold;"));
    }

    layout->addWidget(logo);
    layout->addSpacing(18);

    const QStringList items = {
        QS("\U0001F3E0   Accueil"),
        QS("\U0001F4E6   Produits"),
        QS("\U0001F6E0   \u00c9quipements"),
        QS("\U0001F465   Clients"),
        QS("\U0001F477   Employ\u00e9s")
    };

    for (int i = 0; i < items.size(); ++i)
    {
        QPushButton *b = new QPushButton(items[i]);
        b->setObjectName("nav");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, [this, i]() { goTo(i); });
        layout->addWidget(b);
        navButtons.append(b);
    }

    layout->addStretch();
    layout->addWidget(new BottleWidget);
    layout->addWidget(new QLabel(QS("Smart Water Factory  \u2022  v1.0")), 0, Qt::AlignCenter);

    return sidebar;
}

QWidget *AppWindow::createHomePage()
{
    QWidget *page = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(18);

    QLabel *t = new QLabel(QS("Bienvenue dans Smart Water Factory"));
    t->setObjectName("homeTitle");
    QLabel *s = new QLabel(QS("Choisissez un module pour commencer"));
    s->setObjectName("homeSub");
    layout->addWidget(t);
    layout->addWidget(s);

    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(18);

    for (int i = 0; i < homeTitles.size(); ++i)
    {
        QPushButton *c = new QPushButton;
        c->setObjectName("homeCard");
        c->setCursor(Qt::PointingHandCursor);
        c->setMinimumHeight(150);
        connect(c, &QPushButton::clicked, this, [this, i]() { goTo(i + 1); });
        grid->addWidget(c, i / 2, i % 2);
        homeCards.append(c);
    }

    layout->addLayout(grid);
    layout->addStretch();
    return page;
}

void AppWindow::refreshHome()
{
    for (int i = 0; i < homeCards.size() && i < pages.size(); ++i)
        homeCards[i]->setText(homeTitles[i] + QS("\n\n") + pages[i]->summary());
}

void AppWindow::goTo(int index)
{
    if (index == 0)
        refreshHome();

    stack->setCurrentIndex(index);

    for (int i = 0; i < navButtons.size(); ++i)
        navButtons[i]->setChecked(i == index);
}