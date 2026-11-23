#include "Percepcion.h"
#include "niveles/Nivel.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include <QtMath>

Percepcion::Percepcion(double radio, int radioCeldas) : radio(radio), radioCeldas(radioCeldas) {}

Estimulo Percepcion::percibir(const Nivel *nivel, const QPointF &origen) const
{
    Estimulo e;
    const Carlos *carlos = nivel->getCarlos();
    if (carlos == nullptr)
        return e;

    e.posCarlos = carlos->getPosicion();
    e.celdaCarlos = nivel->celdaDe(e.posCarlos);
    e.direccionCarlos = carlos->getVelocidad();
    e.ruidoCarlos = carlos->getRuido();
    e.panicoCarlos = carlos->getPanico();
    e.carlosEscondido = carlos->estaEscondido();
    e.meta = nivel->getMeta();
    e.tamCelda = nivel->tamCelda();

    // Soldados y linternas que estan cerca del anima o de Carlos
    for (const Soldado *s : nivel->getSoldados()) {
        QPointF p = s->getPosicion();
        QPointF a = p - origen, b = p - e.posCarlos;
        double d = qMin(qSqrt(a.x() * a.x() + a.y() * a.y()), qSqrt(b.x() * b.x() + b.y() * b.y()));
        if (d > radio)
            continue;
        e.posSoldados.append(p);
        e.soldadosPersiguiendo.append(s->getEstado() == EstadoSoldado::PERSIGUIENDO);
        const Linterna *l = s->getLinterna();
        e.zonasIluminadas.append({l->getOrigen(), l->direccionActual(), l->getAlcance(), l->getApertura()});
    }

    // Celdas alrededor de Carlos por donde se puede pasar
    for (int df = -radioCeldas; df <= radioCeldas; ++df) {
        for (int dc = -radioCeldas; dc <= radioCeldas; ++dc) {
            QPoint c(e.celdaCarlos.x() + dc, e.celdaCarlos.y() + df);
            QPointF centro = nivel->centroDe(c);
            if (!nivel->esTransitable(centro))
                continue;
            CeldaPercibida cp;
            cp.celda = c;
            cp.centro = centro;
            cp.escondite = nivel->esEscondite(centro);
            cp.exposicion = nivel->esVegetacion(centro) ? 0.3 : 1.0;
            e.celdasVecinas.append(cp);
        }
    }
    return e;
}
