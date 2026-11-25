#ifndef MIEDOSO_H
#define MIEDOSO_H

#include "Rasgo.h"

// Carlos no es un heroe: es un campesino aterrorizado.
//  - El panico sube cuando hay soldados cerca (y mucho mas si lo persiguen).
//  - Baja si se queda quieto o escondido.
//  - Con panico alto tiembla: se mueve con imprecision y hace mas ruido.
class Miedoso : public Rasgo
{
public:
    Miedoso(double umbralPanico = 60.0, double distanciaMiedo = 320.0,
            double amplitudTemblor = 0.6);

    void aplicar(Carlos *carlos, Nivel *nivel, double dt) override;

    double getUmbralPanico() const { return umbralPanico; }
    void setUmbralPanico(double u) { umbralPanico = qBound(1.0, u, 99.0); }

private:
    double umbralPanico;
    double distanciaMiedo;
    double amplitudTemblor;   // radianes maximos de desvio al caminar
    double fase;              // para que el temblor varie en el tiempo
};

#endif
