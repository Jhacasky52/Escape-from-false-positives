#include "ConocedorDelMonte.h"
#include "entidades/Carlos.h"
#include "niveles/Nivel.h"

ConocedorDelMonte::ConocedorDelMonte(double factorVisibilidad, double tiempoMaxEscondido,
                                     double bonoVelocidadTrocha)
    : Rasgo("Conocedor del monte"), factorVisibilidad(factorVisibilidad),
      tiempoMaxEscondido(tiempoMaxEscondido), bonoVelocidadTrocha(bonoVelocidadTrocha) {}

void ConocedorDelMonte::aplicar(Carlos *carlos, Nivel *nivel, double dt)
{
    Q_UNUSED(dt);
    if (nivel == nullptr)
        return;
    QPointF p = carlos->getPosicion();

    if (carlos->estaEscondido()) {
        carlos->multiplicarVisibilidad(0.0);   // nadie lo ve dentro del escondite
        if (carlos->getTiempoEscondido() >= tiempoMaxEscondido)
            carlos->salirDelEscondite();       // no aguanta mas: tiene que salir
        return;
    }
    if (nivel->esVegetacion(p))
        carlos->multiplicarVisibilidad(factorVisibilidad);
    if (nivel->esTrocha(p))
        carlos->multiplicarVelocidad(bonoVelocidadTrocha);
}
