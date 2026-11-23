#include "Juego.h"
#include "niveles/Nivel.h"
#include "entidades/Carlos.h"
#include "agente/Aprendizaje.h"

Juego::Juego()
    : indiceNivel(0), estado(EstadoJuego::PAUSADO), memoriaAnimas(new Aprendizaje()) {}

Juego::~Juego()
{
    qDeleteAll(niveles);
    niveles.clear();
    delete memoriaAnimas;
}

void Juego::agregarNivel(Nivel *nivel)
{
    if (nivel != nullptr) {
        nivel->setMemoria(memoriaAnimas);
        niveles.append(nivel);
    }
}

void Juego::iniciar()
{
    indiceNivel = 0;
    if (Nivel *n = nivelActual())
        n->cargar();
    estado = EstadoJuego::JUGANDO;
}

void Juego::actualizar(double dt)
{
    if (estado != EstadoJuego::JUGANDO)
        return;

    Nivel *n = nivelActual();
    if (n == nullptr)
        return;

    n->actualizar(dt);

    if (n->haPerdido()) {
        estado = EstadoJuego::CAPTURADO;
        // Las animas recuerdan donde lo atraparon para evitar esa zona la proxima vez
        if (n->getCarlos() != nullptr)
            memoriaAnimas->registrarCaptura(n->celdaDe(n->getCarlos()->getPosicion()));
    }
    else if (n->haGanado())
        estado = (indiceNivel + 1 < niveles.size()) ? EstadoJuego::NIVEL_SUPERADO
                                                    : EstadoJuego::FINAL;
}

void Juego::pasarAlSiguienteNivel()
{
    if (indiceNivel + 1 >= niveles.size()) {
        estado = EstadoJuego::FINAL;
        return;
    }
    ++indiceNivel;
    nivelActual()->cargar();
    estado = EstadoJuego::JUGANDO;
}

void Juego::reiniciarNivel()
{
    if (Nivel *n = nivelActual())
        n->reiniciar();
    estado = EstadoJuego::JUGANDO;
}

void Juego::alternarPausa()
{
    if (estado == EstadoJuego::JUGANDO)
        estado = EstadoJuego::PAUSADO;
    else if (estado == EstadoJuego::PAUSADO)
        estado = EstadoJuego::JUGANDO;
}

Nivel *Juego::nivelActual() const
{
    if (indiceNivel < 0 || indiceNivel >= niveles.size())
        return nullptr;
    return niveles[indiceNivel];
}
