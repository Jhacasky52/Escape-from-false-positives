#include "Entidad.h"

Entidad::Entidad(const QPointF &posicion, const QSizeF &tamano)
    : posicion(posicion), velocidad(0.0, 0.0), tamano(tamano) {}

QRectF Entidad::caja() const
{
    return QRectF(posicion.x() - tamano.width() / 2.0,
                  posicion.y() - tamano.height() / 2.0,
                  tamano.width(), tamano.height());
}

bool Entidad::colisionaCon(const Entidad *otra) const
{
    return otra != nullptr && caja().intersects(otra->caja());
}
