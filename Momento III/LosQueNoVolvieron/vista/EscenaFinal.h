#ifndef ESCENAFINAL_H
#define ESCENAFINAL_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>
#include <QRectF>

// Final del juego: Carlos llega a Cucuta al amanecer y se reencuentra con el
// anima de Ramon. Es una escena corta con tiempos fijos; Enter la adelanta y,
// al terminar, vuelve al menu.
class EscenaFinal : public EscenaJuego
{
    Q_OBJECT
public:
    explicit EscenaFinal(QObject *parent = nullptr);

    void comenzar();   // reinicia la secuencia desde el principio

    static constexpr double DURACION = 24.0;   // s hasta el texto final

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    QPointF posicionCarlos() const;
    QPointF posicionRamon() const;

    double tiempo;
    bool sonoRamon;
    QPixmap spCarlos1, spCarlos2, spRamon, spAnima;
    QVector<QRectF> edificios;        // silueta de la ciudad
    QVector<QPointF> ventanas;        // luces encendidas
};

#endif
