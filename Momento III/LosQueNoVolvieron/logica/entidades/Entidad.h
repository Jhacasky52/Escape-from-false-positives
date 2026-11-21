#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <QPointF>
#include <QSizeF>
#include <QRectF>

// Todo lo que existe en un nivel: personajes, piedras, troncos.
// La posicion es el CENTRO de la entidad, en pixeles del mundo.
class Entidad
{
public:
    Entidad(const QPointF &posicion, const QSizeF &tamano);
    virtual ~Entidad() = default;

    virtual void actualizar(double dt) = 0;

    bool colisionaCon(const Entidad *otra) const;
    QRectF caja() const;   // rectangulo de colision

    QPointF getPosicion() const { return posicion; }
    void setPosicion(const QPointF &p) { posicion = p; }
    QPointF getVelocidad() const { return velocidad; }
    QSizeF getTamano() const { return tamano; }

protected:
    QPointF posicion;
    QPointF velocidad;
    QSizeF tamano;
};

#endif
