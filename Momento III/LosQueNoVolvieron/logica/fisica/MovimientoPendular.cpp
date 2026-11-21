#include "MovimientoPendular.h"
#include <QtMath>

MovimientoPendular::MovimientoPendular(double anguloMax, double longitud, double g)
    : anguloMax(anguloMax), longitud(longitud), g(g) {}

QPointF MovimientoPendular::paso(double dt)
{
    tiempo += dt;
    double th = anguloActual();
    return QPointF(longitud * qSin(th), longitud * qCos(th));
}

double MovimientoPendular::anguloActual() const
{
    return anguloMax * qCos(qSqrt(g / longitud) * tiempo);
}

double MovimientoPendular::periodo() const
{
    return 2.0 * M_PI * qSqrt(longitud / g);
}
