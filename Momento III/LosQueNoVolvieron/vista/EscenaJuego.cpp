#include "EscenaJuego.h"
#include <QKeyEvent>

EscenaJuego::EscenaJuego(QObject *parent)
    : QGraphicsScene(parent), acumulador(0.0), fps(0.0), cuadros(0), tiempoFps(0.0)
{
    temporizador.setTimerType(Qt::PreciseTimer);
    connect(&temporizador, &QTimer::timeout, this, &EscenaJuego::tick);
}

void EscenaJuego::iniciarBucle()
{
    reloj.start();
    acumulador = 0.0;
    temporizador.start(16);
}

void EscenaJuego::detenerBucle()
{
    temporizador.stop();
}

void EscenaJuego::tick()
{
    double transcurrido = reloj.nsecsElapsed() / 1e9;
    reloj.restart();
    if (transcurrido > 0.25)        // evita la "espiral" si el PC se congela
        transcurrido = 0.25;

    acumulador += transcurrido;
    while (acumulador >= DT) {
        actualizarLogica(DT);
        acumulador -= DT;
    }
    actualizarVista();

    // Contador de cuadros por segundo
    ++cuadros;
    tiempoFps += transcurrido;
    if (tiempoFps >= 0.5) {
        fps = cuadros / tiempoFps;
        cuadros = 0;
        tiempoFps = 0.0;
    }
}

void EscenaJuego::keyPressEvent(QKeyEvent *evento)
{
    if (evento->isAutoRepeat())
        return;
    teclas.insert(evento->key());
    teclaPulsada(evento->key());
}

void EscenaJuego::keyReleaseEvent(QKeyEvent *evento)
{
    if (evento->isAutoRepeat())
        return;
    teclas.remove(evento->key());
    teclaSoltada(evento->key());
}
