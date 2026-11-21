#include "MovimientoOscilatorio.h"
#include <QtMath>

MovimientoOscilatorio::MovimientoOscilatorio(double amplitud, double omega, double fase)
    : amplitud(amplitud), omega(omega), fase(fase) {}

QPointF MovimientoOscilatorio::paso(double dt)
{
    tiempo += dt;
    return QPointF(0.0, amplitud * qSin(omega * tiempo + fase));
}
