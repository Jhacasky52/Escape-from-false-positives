#ifndef PERSONAJE_H
#define PERSONAJE_H

#include "Entidad.h"

// Base comun de Carlos y del Soldado.
class Personaje : public Entidad
{
public:
    Personaje(const QPointF &posicion, const QSizeF &tamano, double velocidadBase);

    // Mueve al personaje en la direccion dada (se normaliza) a velocidadBase
    // multiplicada por 'factor' (correr, vegetacion, panico...).
    void mover(const QPointF &direccion, double dt, double factor = 1.0);

    bool estaVivo() const { return vivo; }
    double getVelocidadBase() const { return velocidadBase; }
    double getOrientacion() const { return orientacion; }   // radianes, 0 = derecha

protected:
    double velocidadBase;   // pixeles por segundo
    bool vivo;
    double orientacion;     // hacia donde mira, para girar el sprite
};

#endif
