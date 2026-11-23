#include "Nivel.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "agente/Anima.h"
#include <QtMath>

Nivel::Nivel() : carlos(nullptr), memoria(nullptr), capturado(false) {}

Nivel::~Nivel()
{
    liberar();
}

void Nivel::hacerRuido(const QPointF &origen, double intensidad)
{
    for (Soldado *s : soldados)
        s->escucharRuido(origen, intensidad);
}

bool Nivel::haGanado() const
{
    return carlos != nullptr && meta.contains(carlos->getPosicion());
}

bool Nivel::haPerdido() const
{
    return capturado;
}

void Nivel::reiniciar()
{
    liberar();
    capturado = false;
    cargar();
}

void Nivel::liberar()
{
    delete carlos;
    carlos = nullptr;
    qDeleteAll(soldados);
    soldados.clear();
    qDeleteAll(animas);
    animas.clear();
}

void Nivel::actualizarAnimas(double dt)
{
    for (Anima *a : animas)
        a->actualizar(dt, this);
}

QPoint Nivel::celdaDe(const QPointF &p) const
{
    return QPoint(qFloor(p.x() / tamCelda()), qFloor(p.y() / tamCelda()));
}

QPointF Nivel::centroDe(const QPoint &celda) const
{
    return QPointF((celda.x() + 0.5) * tamCelda(), (celda.y() + 0.5) * tamCelda());
}
