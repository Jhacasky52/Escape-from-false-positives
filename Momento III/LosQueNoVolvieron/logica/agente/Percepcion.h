#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "Estimulo.h"

class Nivel;

// 1. PERCEPCION: lee del nivel lo que pasa dentro de un radio y arma un Estimulo.
class Percepcion
{
public:
    explicit Percepcion(double radio = 520.0, int radioCeldas = 3);

    Estimulo percibir(const Nivel *nivel, const QPointF &origen) const;

    double getRadio() const { return radio; }
    void setRadio(double r) { radio = r; }

private:
    double radio;       // px alrededor del anima
    int radioCeldas;    // celdas alrededor de Carlos que se evaluan
};

#endif
