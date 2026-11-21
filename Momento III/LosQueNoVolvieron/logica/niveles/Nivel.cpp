#include "Nivel.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"

Nivel::Nivel() : carlos(nullptr), capturado(false) {}

Nivel::~Nivel()
{
    liberar();
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
}
