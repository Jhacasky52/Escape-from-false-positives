#ifndef ANIMARAMON_H
#define ANIMARAMON_H

#include "Anima.h"
#include <QString>

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

    // Recuerdos de Carlos sobre Ramon: se muestran cuando su luz aparece
    static int numeroRecuerdos();
    static QString recuerdo(int indice);   // indice se toma modulo numeroRecuerdos()
    static QString siguienteRecuerdo();    // va uno por uno sin repetir hasta dar la vuelta
    bool enReencuentro() const { return apareceAlFinal; }
    void setPeriodo(double p) { periodo = p; ciclo = qMax(0.0, p - 8.0); }
    double getPeriodo() const { return periodo; }

private:
    bool visible;
    bool apareceAlFinal;
    double ciclo;
    double periodo;   // s entre apariciones (lo cambia la dificultad)

    static constexpr double PERIODO = 30.0;     // s entre apariciones
    static constexpr double DURACION = 6.0;     // s que se deja ver
};

#endif
