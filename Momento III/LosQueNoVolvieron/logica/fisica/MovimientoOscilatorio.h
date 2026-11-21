#ifndef MOVIMIENTOOSCILATORIO_H
#define MOVIMIENTOOSCILATORIO_H

#include "ModeloFisico.h"

// Oscilacion armonica: troncos en el oleaje (nivel 2) y vaiven de las animas.
//   y(t) = A sin(omega t + fase)
// paso() devuelve QPointF(0, y): solo desplaza en vertical.
class MovimientoOscilatorio : public ModeloFisico
{
public:
    MovimientoOscilatorio(double amplitud, double omega, double fase = 0.0);

    QPointF paso(double dt) override;

    void setAmplitud(double a) { amplitud = a; }
    double getAmplitud() const { return amplitud; }

private:
    double amplitud;
    double omega;
    double fase;
};

#endif
