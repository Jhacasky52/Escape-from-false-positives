#include "Accion.h"
#include "Anima.h"
#include "entidades/Carlos.h"
#include <QtMath>

Accion::Accion() {}

void Accion::ejecutar(const Decision &d, Anima *anima, Carlos *carlos, double insistencia, double dt)
{
    // Cada anima se ubica un poco desplazada para que no se encimen
    QPointF objetivo = d.destino + anima->getDesplazamiento();

    // Con mas insistencia se adelanta menos: se queda mas cerca de Carlos
    if (d.tipo == TipoDecision::GUIAR && carlos != nullptr) {
        QPointF haciaCarlos = carlos->getPosicion() - objetivo;
        objetivo += haciaCarlos * (0.35 * insistencia);
    }
    if (d.tipo == TipoDecision::CALMAR || d.tipo == TipoDecision::OCULTARSE)
        objetivo = d.destino + anima->getDesplazamiento() * 1.4;

    // Movimiento suave hacia el objetivo
    QPointF delta = objetivo - anima->getPosicion();
    double distancia = qSqrt(delta.x() * delta.x() + delta.y() * delta.y());
    double velocidad = VELOCIDAD * (1.0 + 0.6 * d.urgencia);
    if (distancia > 1.0)
        anima->mover(delta / distancia * qMin(distancia, velocidad * dt));

    // Color y brillo
    double t = anima->getTiempo();
    switch (d.tipo) {
    case TipoDecision::GUIAR:
        anima->setColor(ColorAnima::BLANCA);
        // Si Carlos la ignora, parpadea con mas fuerza para llamar su atencion
        anima->setBrillo(0.75 + 0.25 * insistencia * qSin(t * 8.0));
        break;
    case TipoDecision::ADVERTIR: {
        anima->setColor(ColorAnima::ROJA);
        double frecuencia = 6.0 + 8.0 * d.urgencia;
        anima->setBrillo(0.55 + 0.45 * qAbs(qSin(t * frecuencia)));
        break;
    }
    case TipoDecision::OCULTARSE:
        anima->setColor(ColorAnima::TENUE);
        anima->setBrillo(0.2);
        break;
    case TipoDecision::CALMAR:
        anima->setColor(ColorAnima::BLANCA);
        anima->setBrillo(0.9);
        break;
    }

    // Su presencia calma a Carlos (mas si la decision es CALMAR)
    if (carlos != nullptr && d.tipo != TipoDecision::OCULTARSE) {
        QPointF dc = carlos->getPosicion() - anima->getPosicion();
        double dist = qSqrt(dc.x() * dc.x() + dc.y() * dc.y());
        if (dist < RADIO_CALMA)
            carlos->calmar((d.tipo == TipoDecision::CALMAR ? 9.0 : 2.0) * dt);
    }
}
