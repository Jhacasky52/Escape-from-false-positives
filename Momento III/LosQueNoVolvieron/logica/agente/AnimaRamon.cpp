#include "AnimaRamon.h"
#include "niveles/Nivel.h"
#include "entidades/Carlos.h"

AnimaRamon::AnimaRamon(const QPointF &posicion, Aprendizaje *memoria)
    : Anima(posicion, memoria, 3), visible(false), apareceAlFinal(false), ciclo(PERIODO - 8.0)
{
    color = ColorAnima::RAMON;
}

void AnimaRamon::actualizar(double dt, Nivel *nivel)
{
    Anima::actualizar(dt, nivel);   // piensa y actua como cualquier anima
    color = ColorAnima::RAMON;      // pero siempre con su luz calida

    if (apareceAlFinal) {
        visible = true;
        brillo = 1.0;
        return;
    }
    ciclo += dt;
    if (ciclo >= PERIODO)
        ciclo -= PERIODO;
    bool aterrorizado = nivel->getCarlos() != nullptr && nivel->getCarlos()->getPanico() > 85.0;
    visible = ciclo < DURACION || aterrorizado;
    if (!visible)
        brillo = 0.0;
}

void AnimaRamon::reencuentro()
{
    apareceAlFinal = true;
    visible = true;
    brillo = 1.0;
}
