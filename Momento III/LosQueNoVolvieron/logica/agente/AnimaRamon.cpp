#include "AnimaRamon.h"
#include "niveles/Nivel.h"
#include "entidades/Carlos.h"

AnimaRamon::AnimaRamon(const QPointF &posicion, Aprendizaje *memoria)
    : Anima(posicion, memoria, 3), visible(false), apareceAlFinal(false), ciclo(PERIODO - 8.0), periodo(PERIODO)
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
    if (ciclo >= periodo)
        ciclo -= periodo;
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

namespace {
// Lo que Carlos recuerda de Ramon (no habla: es la memoria de Carlos)
const char *const RECUERDOS[] = {
    "Ramón silbaba así cuando volvían del cultivo.",
    "\"Por la trocha, compa, que el monte nos cuida.\"",
    "Ramón le prometió a su mamá que volvía en ocho días.",
    "De niños cruzaban el río en invierno, agarrados de las rocas.",
    "\"Si un día me pasa algo, no se quede callado.\"",
    "Ramón quería comprar una moto para sacar el cacao al pueblo.",
};
}

int AnimaRamon::numeroRecuerdos()
{
    return static_cast<int>(sizeof(RECUERDOS) / sizeof(RECUERDOS[0]));
}

QString AnimaRamon::recuerdo(int indice)
{
    int n = numeroRecuerdos();
    return QString::fromUtf8(RECUERDOS[((indice % n) + n) % n]);
}

QString AnimaRamon::siguienteRecuerdo()
{
    static int indice = 0;   // compartido entre niveles: no se repiten al pasar al rio
    return recuerdo(indice++);
}
