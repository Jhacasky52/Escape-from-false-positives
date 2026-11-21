#include "Carlos.h"

Carlos::Carlos(const QPointF &posicion)
    : Personaje(posicion, QSizeF(48, 48), VELOCIDAD_CAMINAR),
      corriendo(false), panico(0.0), ruido(0.0), aire(100.0) {}

void Carlos::actualizar(double dt)
{
    double factor = corriendo ? FACTOR_CORRER : 1.0;
    mover(direccion, dt, factor);

    // Ruido provisional: correr hace mas ruido que caminar (se usa en la fase 3)
    if (!estaMoviendose())
        ruido = 0.0;
    else
        ruido = corriendo ? 1.0 : 0.3;

    if (!limites.isNull()) {
        posicion.setX(qBound(limites.left(), posicion.x(), limites.right()));
        posicion.setY(qBound(limites.top(), posicion.y(), limites.bottom()));
    }
}

bool Carlos::estaMoviendose() const
{
    return !direccion.isNull();
}
