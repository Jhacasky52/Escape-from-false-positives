#ifndef ESCENAINTRO_H
#define ESCENAINTRO_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>
#include <QPointF>

// Introduccion: quien era Ramon y por que huye Carlos.
// Escena corta por bloques de texto; Enter pasa al siguiente bloque y, al
// final, empieza el nivel 1 (senal terminada()).
class EscenaIntro : public EscenaJuego
{
    Q_OBJECT
public:
    explicit EscenaIntro(QObject *parent = nullptr);

    void comenzar();

signals:
    void terminada();

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    double tiempo;
    bool sonoRamon;
    QPixmap spCarlos1, spCarlos2, spRamon1, spRamon2, spAnimaRamon;
    QVector<QPointF> luciernagas;
};

#endif
