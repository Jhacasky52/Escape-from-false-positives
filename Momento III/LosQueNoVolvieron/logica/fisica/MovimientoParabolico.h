#ifndef MOVIMIENTOPARABOLICO_H
#define MOVIMIENTOPARABOLICO_H

#include "ModeloFisico.h"

// Tiro parabolico de las piedras (nivel 1).
//   x(t) = v0 * cos(angulo) * t
//   z(t) = v0 * sin(angulo) * t - 1/2 * g * t^2
// paso() devuelve QPointF(x, z): x = distancia recorrida en el suelo,
// z = altura sobre el suelo. Como el nivel es cenital, quien lo use
// convierte x a la direccion del lanzamiento y z al tamano del sprite.
class MovimientoParabolico : public ModeloFisico
{
public:
    MovimientoParabolico(double v0, double angulo, double g = 600.0);

    QPointF paso(double dt) override;

    double altura() const;          // z actual
    double distancia() const;       // x actual
    bool aterrizo() const;          // true cuando z vuelve a 0
    double tiempoDeVuelo() const;   // 2 v0 sin(angulo) / g
    double alcanceMaximo() const;   // v0^2 sin(2 angulo) / g

private:
    double v0;
    double angulo;
    double g;
};

#endif
