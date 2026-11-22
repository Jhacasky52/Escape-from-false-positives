#include "Piedra.h"
#include "fisica/MovimientoParabolico.h"
#include <QtMath>

Piedra::Piedra(const QPointF &origen, double direccion, double v0, double elevacion)
    : Entidad(origen, QSizeF(10, 10)), origen(origen), direccion(direccion),
      altura(0.0), activa(true), detenida(false), intensidad(0.0),
      fisica(new MovimientoParabolico(v0, elevacion))
{
    // Una piedra lanzada con mas fuerza suena mas fuerte al caer
    intensidad = qBound(0.4, v0 / 500.0, 1.0);
}

Piedra::~Piedra()
{
    delete fisica;
}

void Piedra::actualizar(double dt)
{
    if (detenida || fisica->aterrizo())
        return;
    QPointF xz = fisica->paso(dt);   // x = distancia en el suelo, z = altura
    QPointF anterior = posicion;
    posicion = origen + QPointF(qCos(direccion), qSin(direccion)) * xz.x();
    velocidad = (posicion - anterior) / dt;
    altura = xz.y();
}

bool Piedra::cayo() const
{
    return detenida || fisica->aterrizo();
}

void Piedra::detener()
{
    detenida = true;
    altura = 0.0;
    velocidad = QPointF();
}
