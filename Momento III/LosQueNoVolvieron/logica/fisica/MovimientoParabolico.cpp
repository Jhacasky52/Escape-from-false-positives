#include "MovimientoParabolico.h"
#include <QtMath>

MovimientoParabolico::MovimientoParabolico(double v0, double angulo, double g)
    : v0(v0), angulo(angulo), g(g) {}

QPointF MovimientoParabolico::paso(double dt)
{
    tiempo += dt;
    if (tiempo > tiempoDeVuelo())
        tiempo = tiempoDeVuelo();   // no atraviesa el suelo
    return QPointF(distancia(), altura());
}

double MovimientoParabolico::distancia() const
{
    return v0 * qCos(angulo) * tiempo;
}

double MovimientoParabolico::altura() const
{
    double z = v0 * qSin(angulo) * tiempo - 0.5 * g * tiempo * tiempo;
    return z > 0.0 ? z : 0.0;
}

bool MovimientoParabolico::aterrizo() const
{
    return tiempo >= tiempoDeVuelo();
}

double MovimientoParabolico::tiempoDeVuelo() const
{
    return 2.0 * v0 * qSin(angulo) / g;
}

double MovimientoParabolico::alcanceMaximo() const
{
    return v0 * v0 * qSin(2.0 * angulo) / g;
}
