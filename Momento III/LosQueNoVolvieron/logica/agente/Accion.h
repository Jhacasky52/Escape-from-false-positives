#ifndef ACCION_H
#define ACCION_H

#include "Decision.h"

class Anima;
class Carlos;

// 3. ACCION: ejecuta la decision. Mueve el anima hacia el destino, cambia su
// color y brillo (blanco = seguro, rojo = peligro, tenue = ocultarse) y,
// si esta cerca de Carlos, le baja el panico.
class Accion
{
public:
    Accion();

    void ejecutar(const Decision &d, Anima *anima, Carlos *carlos, double insistencia, double dt);

    static constexpr double VELOCIDAD = 130.0;      // px/s
    static constexpr double RADIO_CALMA = 90.0;     // px
};

#endif
