#ifndef RESPONSIVE_H
#define RESPONSIVE_H

// Adaptation de la taille et des polices a la resolution de l'ecran.
// Le design de reference est 1600 x 900 (pixels logiques Qt, DPI deja pris en charge par Qt).
// Sur un ecran plus petit, polices (px/pt des feuilles de style), tailles fixes/minimales
// et tailles de fenetre sont reduites ; sur un grand ecran, elles sont agrandies jusqu'a 125 %.

#include <QGuiApplication>
#include <QScreen>
#include <QWidget>
#include <QString>
#include <QRegularExpression>
#include <QList>
#include <QVariant>
#include <QRect>
#include <QtGlobal>
#include <algorithm>

namespace Responsive {

inline qreal factor()
{
    static qreal f = -1.0;
    if (f < 0.0) {
        const QScreen *s = QGuiApplication::primaryScreen();
        if (!s) {
            f = 1.0;
        } else {
            const QSize a = s->availableGeometry().size();
            f = qBound<qreal>(0.7, qMin(a.width() / 1600.0, a.height() / 900.0), 1.25);
        }
        if (qAbs(f - 1.0) < 0.03) f = 1.0;
    }
    return f;
}

// Multiplie toutes les valeurs "NNpx" / "NNpt" d'une feuille de style.
inline QString scaleStyle(const QString &css)
{
    const qreal f = factor();
    if (f == 1.0 || css.isEmpty()) return css;
    static const QRegularExpression re(QStringLiteral("(\\d+(?:\\.\\d+)?)(px|pt)\\b"));
    QString out;
    int last = 0;
    auto it = re.globalMatch(css);
    while (it.hasNext()) {
        const auto m = it.next();
        out += css.mid(last, m.capturedStart() - last);
        const double v = m.captured(1).toDouble();
        const bool pt = (m.captured(2) == QLatin1String("pt"));
        int n = (v <= 1.0) ? int(v) : qRound(v * f);   // les filets de 1 px restent a 1 px
        if (v > 1.0) n = qMax(n, pt ? 6 : 2);
        out += QString::number(n) + m.captured(2);
        last = m.capturedEnd();
    }
    out += css.mid(last);
    return out;
}

// A appeler une fois apres la construction d'une fenetre ou d'une page.
inline void apply(QWidget *root)
{
    if (!root) return;
    const qreal f = factor();

    if (f != 1.0) {
        QList<QWidget *> all = root->findChildren<QWidget *>();
        all.prepend(root);
        for (QWidget *w : std::as_const(all)) {
            if (w->property("_responsive").toBool()) continue;     // evite un double redimensionnement
            w->setProperty("_responsive", true);

            const QString ss = w->styleSheet();
            if (!ss.isEmpty()) w->setStyleSheet(scaleStyle(ss));

            const QSize mn = w->minimumSize(), mx = w->maximumSize();
            w->setMinimumSize(mn.width() > 0 ? qRound(mn.width() * f) : 0,
                              mn.height() > 0 ? qRound(mn.height() * f) : 0);
            w->setMaximumSize(mx.width() < QWIDGETSIZE_MAX ? qRound(mx.width() * f) : QWIDGETSIZE_MAX,
                              mx.height() < QWIDGETSIZE_MAX ? qRound(mx.height() * f) : QWIDGETSIZE_MAX);
        }
    }

    // Une fenetre ne doit jamais depasser l'ecran disponible.
    if (root->isWindow()) {
        if (const QScreen *s = QGuiApplication::primaryScreen()) {
            const QRect a = s->availableGeometry();
            root->setMinimumSize(qMin(root->minimumWidth(), a.width()),
                                 qMin(root->minimumHeight(), a.height()));
            root->resize(qMin(root->width(), int(a.width() * 0.96)),
                         qMin(root->height(), int(a.height() * 0.94)));
        }
    }
}

} // namespace Responsive

#endif // RESPONSIVE_H
