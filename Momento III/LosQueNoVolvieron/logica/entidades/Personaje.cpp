#include "Personaje.h"
#include <QtMath>

Personaje::Personaje(const QPointF &posicion, const QSizeF &tamano, double velocidadBase)
    : Entidad(posicion, tamano), velocidadBase(velocidadBase), vivo(true), orientacion(-M_PI / 2.0) {}

void Personaje::mover(const QPointF &direccion, double dt, double factor)
{
    double largo = qSqrt(direccion.x() * direccion.x() + direccion.y() * direccion.y());
    if (largo < 1e-9) {
        velocidad = QPointF(0.0, 0.0);
        return;
    }
    QPointF unitaria = direccion / largo;
    velocidad = unitaria * velocidadBase * factor;
    posicion += velocidad * dt;
    orientacion = qAtan2(unitaria.y(), unitaria.x());
}
