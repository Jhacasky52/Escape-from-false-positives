#ifndef MOVIMIENTOFLUIDO_H
#define MOVIMIENTOFLUIDO_H

#include "ModeloFisico.h"

// Movimiento de Carlos dentro del rio (nivel 2):
//   m dv/dt = F_carlos + k (v_rio - v) - b v
// - F_carlos: fuerza que hace Carlos (setFuerzaExterna), en x y en y.
// - k (v_rio - v): empuje de la corriente; v_rio va solo en x (rio abajo).
// - b v: resistencia del agua, proporcional a la velocidad.
// Se integra con Euler semi-implicito. paso() devuelve la posicion
// acumulada desde que Carlos entro al agua (o desde reiniciar()).
class MovimientoFluido : public ModeloFisico
{
public:
    MovimientoFluido(double masa, double k, double b, double velocidadRio);

    QPointF paso(double dt) override;
    void reiniciar() override;

    void setFuerzaExterna(const QPointF &f) { fuerzaExterna = f; }
    void setVelocidadRio(double v) { velocidadRio = v; }
    void setVelocidad(const QPointF &v) { velocidad = v; }

    QPointF getVelocidad() const { return velocidad; }
    double getVelocidadRio() const { return velocidadRio; }
    // Velocidad a la que se estabiliza el cuerpo (aceleracion = 0)
    QPointF velocidadTerminal() const;

private:
    double masa;
    double k;
    double b;
    double velocidadRio;
    QPointF fuerzaExterna;
    QPointF velocidad;
    QPointF posicion;
};

#endif
