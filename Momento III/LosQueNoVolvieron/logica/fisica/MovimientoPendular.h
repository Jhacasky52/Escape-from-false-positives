#ifndef MOVIMIENTOPENDULAR_H
#define MOVIMIENTOPENDULAR_H

#include "ModeloFisico.h"

// Barrido de las linternas (niveles 1 y 2), pendulo de angulo pequeno:
//   theta(t) = theta0 * cos( sqrt(g / L) * t )
// paso() devuelve la punta del pendulo relativa al pivote:
//   QPointF(L sin(theta), L cos(theta))
// La linterna usa anguloActual() para girar su cono de luz.
// g y L solo fijan el periodo del barrido (con g = 9.8 y L = 4 el periodo
// es de unos 4 s), por eso aqui van en metros y no en pixeles.
class MovimientoPendular : public ModeloFisico
{
public:
    MovimientoPendular(double anguloMax, double longitud, double g = 9.8);

    QPointF paso(double dt) override;

    double anguloActual() const;   // radianes
    double periodo() const;        // 2 pi sqrt(L / g)

private:
    double anguloMax;
    double longitud;
    double g;
};

#endif
