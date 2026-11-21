#include "MovimientoFluido.h"

MovimientoFluido::MovimientoFluido(double masa, double k, double b, double velocidadRio)
    : masa(masa), k(k), b(b), velocidadRio(velocidadRio) {}

QPointF MovimientoFluido::paso(double dt)
{
    tiempo += dt;
    QPointF vRio(velocidadRio, 0.0);
    QPointF fuerza = fuerzaExterna + k * (vRio - velocidad) - b * velocidad;
    QPointF aceleracion = fuerza / masa;
    velocidad += aceleracion * dt;
    posicion += velocidad * dt;
    return posicion;
}

void MovimientoFluido::reiniciar()
{
    ModeloFisico::reiniciar();
    velocidad = QPointF();
    posicion = QPointF();
}

QPointF MovimientoFluido::velocidadTerminal() const
{
    // 0 = F + k (vr - v) - b v  ->  v = (F + k vr) / (k + b)
    QPointF vRio(velocidadRio, 0.0);
    return (fuerzaExterna + k * vRio) / (k + b);
}
