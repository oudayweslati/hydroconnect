#ifndef LIVRAISON_DONUTWIDGET_H
#define LIVRAISON_DONUTWIDGET_H
#include <QWidget>
#include <QVector>

// Graphique en anneau : valeurs dans l'ordre Livrées, En cours, En retard, Planifiées.
class LivraisonDonutWidget : public QWidget {
    Q_OBJECT
public:
    explicit LivraisonDonutWidget(QWidget *parent = nullptr);
    void setData(const QVector<int> &values);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<int> m_values = {0, 0, 0, 0};
};
#endif
