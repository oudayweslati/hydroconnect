#include "livraison_donutwidget.h"
#include <QPainter>

LivraisonDonutWidget::LivraisonDonutWidget(QWidget *parent) : QWidget(parent) { setMinimumSize(130,130); }

void LivraisonDonutWidget::setData(const QVector<int> &values)
{
    m_values = values;
    while (m_values.size() < 4) m_values.append(0);
    update();
}

void LivraisonDonutWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int side = qMin(width(),height()) - 12;
    QRectF r((width()-side)/2.0,(height()-side)/2.0,side,side);
    int total = 0; for (int v : m_values) total += v;
    const QColor colors[4] = {QColor("#10B981"),QColor("#F59E0B"),QColor("#EF4444"),QColor("#2878F0")};
    p.setPen(Qt::NoPen);
    if (total <= 0) {
        p.setBrush(QColor("#E5EDF7")); p.drawEllipse(r);
    } else {
        int start = 90*16;
        for (int i = 0; i < 4; ++i) {
            int span = int(-360.0*16*m_values[i]/total);
            p.setBrush(colors[i]); p.drawPie(r, start, span); start += span;
        }
    }
    qreal inset = side*0.29;
    QRectF inner(r.x()+inset,r.y()+inset,r.width()-2*inset,r.height()-2*inset);
    p.setBrush(Qt::white); p.drawEllipse(inner);
    p.setPen(QColor("#31547E")); QFont f=p.font(); f.setPointSize(8); p.setFont(f);
    p.drawText(inner,Qt::AlignCenter,QString("Total\n%1 livraison(s)").arg(total));
}
