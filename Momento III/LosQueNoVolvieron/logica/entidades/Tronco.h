#ifndef TRONCO_H
#define TRONCO_H

#include "Entidad.h"

class MovimientoOscilatorio;

enum class TipoTronco { TRONCO, ESCOMBROS };

// Tronco o escombros que bajan flotando por el rio (nivel 2).
// Avanzan con la corriente y suben y bajan con el oleaje:
//   y(t) = superficie + A sin(omega t + fase)   (MovimientoOscilatorio)
// Si golpean a Carlos lo empujan rio abajo.
class Tronco : public Entidad
{
public:
    Tronco(const QPointF &posicion, TipoTronco tipo, double fase);
    ~Tronco() override;

    void actualizar(double dt) override;

    // El nivel le pasa cada cuadro el nivel del agua y la corriente
    void setAgua(double superficie, double velocidadRio);

    TipoTronco getTipo() const { return tipo; }
    double getInclinacion() const { return inclinacion; }   // radianes, para dibujar

private:
    TipoTronco tipo;
    MovimientoOscilatorio *oleaje;
    double superficie;
    double velocidadRio;
    double inclinacion;
};

#endif
