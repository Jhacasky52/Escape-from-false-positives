#include "Miedoso.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "niveles/Nivel.h"
#include <QtMath>

Miedoso::Miedoso(double umbralPanico, double distanciaMiedo, double amplitudTemblor)
    : Rasgo("Miedoso"), umbralPanico(umbralPanico), distanciaMiedo(distanciaMiedo),
      amplitudTemblor(amplitudTemblor), fase(0.0) {}

void Miedoso::aplicar(Carlos *carlos, Nivel *nivel, double dt)
{
    if (nivel == nullptr)
        return;
    fase += dt;

    // 1. Cuanto miedo da lo que tiene alrededor
    double estimulo = 0.0;
    for (const Soldado *s : nivel->getSoldados()) {
        QPointF d = s->getPosicion() - carlos->getPosicion();
        double distancia = qSqrt(d.x() * d.x() + d.y() * d.y());
        if (distancia < distanciaMiedo) {
            double cercania = 1.0 - distancia / distanciaMiedo;   // 0 lejos, 1 encima
            double peso = (s->getEstado() == EstadoSoldado::PERSIGUIENDO) ? 45.0 : 22.0;
            estimulo = qMax(estimulo, cercania * peso);
        }
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO)
            estimulo = qMax(estimulo, 18.0);   // saber que lo persiguen ya da miedo
    }

    if (estimulo > 0.0) {
        carlos->aumentarPanico(estimulo * dt);
    } else {
        // 2. Se calma poco a poco, mas rapido si esta quieto o escondido
        double calma = 3.0;
        if (carlos->estaEscondido())
            calma = 12.0;
        else if (!carlos->estaMoviendose())
            calma = 8.0;
        carlos->calmar(calma * dt);
    }

    // 3. Efectos del panico alto: temblor y ruido
    double panico = carlos->getPanico();
    if (panico > umbralPanico) {
        double exceso = (panico - umbralPanico) / (100.0 - umbralPanico);   // 0..1
        // Desvio que oscila rapido: el movimiento se vuelve impreciso
        double desvio = amplitudTemblor * exceso * qSin(fase * 13.0) * qCos(fase * 7.3);
        carlos->setTemblor(desvio);
        carlos->multiplicarRuido(1.0 + exceso);
    }
}
