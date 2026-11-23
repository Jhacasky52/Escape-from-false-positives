#ifndef ANIMARAMON_H
#define ANIMARAMON_H

#include "Anima.h"

// Ramon, el amigo de Carlos: una luz mas calida que solo aparece por momentos
// (cada cierto tiempo o cuando Carlos esta aterrorizado). Al final del juego
// se aparece por ultima vez para el reencuentro.
class AnimaRamon : public Anima
{
public:
    AnimaRamon(const QPointF &posicion, Aprendizaje *memoria);

    void actualizar(double dt, Nivel *nivel) override;
    bool esVisible() const override { return visible; }

    void reencuentro();
    bool enReencuentro() const { return apareceAlFinal; }

private:
    bool visible;
    bool apareceAlFinal;
    double ciclo;

    static constexpr double PERIODO = 30.0;     // s entre apariciones
    static constexpr double DURACION = 6.0;     // s que se deja ver
};

#endif
