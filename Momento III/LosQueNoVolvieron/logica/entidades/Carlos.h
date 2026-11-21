#ifndef CARLOS_H
#define CARLOS_H

#include "Personaje.h"

// Protagonista. En la fase 1 solo camina y corre; el panico, el ruido,
// el aire, los escondites y los rasgos llegan en la fase 3 y la fisica
// del agua (MovimientoFluido) en la fase 5.
class Carlos : public Personaje
{
public:
    explicit Carlos(const QPointF &posicion);

    void actualizar(double dt) override;

    // Entrada del jugador: direccion deseada (sin normalizar)
    void setDireccion(const QPointF &d) { direccion = d; }
    void correr(bool activo) { corriendo = activo; }

    bool estaCorriendo() const { return corriendo; }
    bool estaMoviendose() const;

    double getPanico() const { return panico; }
    double getRuido() const { return ruido; }
    double getAire() const { return aire; }

    // Pasa los limites del nivel para que no se salga del mapa
    void setLimites(const QRectF &l) { limites = l; }

private:
    QPointF direccion;
    QRectF limites;
    bool corriendo;
    double panico;   // 0..100   (fase 3)
    double ruido;    // 0..1     (fase 3)
    double aire;     // 0..100   (fase 5)

    static constexpr double VELOCIDAD_CAMINAR = 140.0;   // px/s
    static constexpr double FACTOR_CORRER = 1.8;
};

#endif
