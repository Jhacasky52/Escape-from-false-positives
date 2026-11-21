#ifndef MODELOFISICO_H
#define MODELOFISICO_H

#include <QPointF>

// Clase base de todas las fisicas del juego.
// Convencion: paso(dt) avanza el tiempo interno dt segundos y devuelve la
// posicion del cuerpo RELATIVA al origen del movimiento (en pixeles).
// Unidades: pixeles, segundos, radianes.
class ModeloFisico
{
public:
    ModeloFisico();
    virtual ~ModeloFisico() = default;

    virtual QPointF paso(double dt) = 0;
    virtual void reiniciar();

    double getTiempo() const { return tiempo; }

protected:
    double tiempo;
};

#endif
