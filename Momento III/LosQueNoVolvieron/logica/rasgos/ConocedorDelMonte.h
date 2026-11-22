#ifndef CONOCEDORDELMONTE_H
#define CONOCEDORDELMONTE_H

#include "Rasgo.h"

// Carlos crecio en la vereda y conoce la selva:
//  - dentro de la vegetacion los soldados lo ven mucho menos,
//  - puede quedarse en un escondite (tronco hueco, matorral) un tiempo limitado,
//  - se mueve mas rapido por la trocha.
class ConocedorDelMonte : public Rasgo
{
public:
    ConocedorDelMonte(double factorVisibilidad = 0.35,
                      double tiempoMaxEscondido = 6.0,
                      double bonoVelocidadTrocha = 1.25);

    void aplicar(Carlos *carlos, Nivel *nivel, double dt) override;

    double getTiempoMaxEscondido() const { return tiempoMaxEscondido; }

private:
    double factorVisibilidad;
    double tiempoMaxEscondido;
    double bonoVelocidadTrocha;
};

#endif
