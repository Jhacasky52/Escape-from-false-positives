#include "EscenaJuego.h"
#include <QKeyEvent>
#include <QColor>
#include <QPainter>
#include "Sonidos.h"

EscenaJuego::EscenaJuego(QObject *parent)
    : QGraphicsScene(parent), sonidos(nullptr), acumulador(0.0), fps(0.0), cuadros(0), tiempoFps(0.0), tiempoAviso(0.0)
{
    setBackgroundBrush(QColor(8, 10, 8));   // fondo por defecto (fuera del mapa)
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
    if (tiempoAviso > 0.0)
        tiempoAviso -= transcurrido;
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

void EscenaJuego::mostrarAviso(const QString &texto, double segundos)
{
    aviso = texto;
    tiempoAviso = segundos;
    update();
}

void EscenaJuego::sonar(const QString &nombre, double volumen)
{
    if (sonidos != nullptr)
        sonidos->reproducir(nombre, volumen);
}

void EscenaJuego::dibujarAviso(QPainter *painter, int ancho, int alto)
{
    Q_UNUSED(alto);
    if (tiempoAviso <= 0.0 || aviso.isEmpty())
        return;
    painter->save();
    QFont f = painter->font();
    f.setPointSize(13);
    f.setBold(true);
    painter->setFont(f);
    QRect caja(ancho / 2 - 160, 80, 320, 36);
    painter->fillRect(caja, QColor(0, 0, 0, 190));
    painter->setPen(QColor(235, 225, 170, int(255 * qMin(1.0, tiempoAviso))));
    painter->drawText(caja, Qt::AlignCenter, aviso);
    painter->restore();
}
