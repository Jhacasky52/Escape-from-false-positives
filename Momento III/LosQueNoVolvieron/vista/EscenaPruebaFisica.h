#ifndef ESCENAPRUEBAFISICA_H
#define ESCENAPRUEBAFISICA_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>

class Carlos;
class MovimientoParabolico;
class MovimientoPendular;
class MovimientoFluido;
class MovimientoOscilatorio;
class QGraphicsPixmapItem;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;

// Escena de prueba de la fase 1: muestra las cuatro fisicas funcionando.
//  - Zona selva (arriba a la izquierda): Carlos se mueve con WASD/flechas,
//    Shift corre, mantener Espacio carga y soltar lanza una piedra (parabolico).
//    Un soldado barre con la linterna (pendulo).
//  - Zona rio (abajo): un cuerpo arrastrado por la corriente (fluido con
//    arrastre; mantener J para nadar contra la corriente) y un tronco que
//    sube y baja con el oleaje (oscilatorio).
//  - Panel derecho: las animas flotando con vaiven (oscilatorio) y los
//    valores de cada fisica en tiempo real.
class EscenaPruebaFisica : public EscenaJuego
{
    Q_OBJECT
public:
    explicit EscenaPruebaFisica(QObject *parent = nullptr);
    ~EscenaPruebaFisica() override;

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void teclaSoltada(int tecla) override;

private:
    void construirSelva();
    void construirRio();
    void construirPanel();
    void lanzarPiedra();
    void reiniciarCuerpoRio();

    // ---- Logica ----
    Carlos *carlos;
    MovimientoParabolico *tiro;        // nullptr si no hay piedra en el aire
    QPointF origenTiro;
    QPointF direccionTiro;
    bool cargando;
    double carga;                      // 0..1 segun el tiempo con Espacio

    MovimientoPendular *pendulo;
    QPointF posSoldado;

    MovimientoFluido *fluido;
    QPointF origenCuerpo;
    QPointF posCuerpo;

    MovimientoOscilatorio *oleaje;
    double xTronco;
    QVector<MovimientoOscilatorio *> vaivenes;   // uno por anima
    double tiempoAnimacion;

    // ---- Vista ----
    QPixmap spCarlosIdle, spCarlosWalk1, spCarlosWalk2, spCarlosRun, spCarlosThrow;
    QPixmap spPiedra[3];
    QGraphicsPixmapItem *itemCarlos;
    QGraphicsPixmapItem *itemPiedra;
    QGraphicsEllipseItem *sombraPiedra;
    QGraphicsPixmapItem *itemSoldado;
    QGraphicsPixmapItem *itemCono;
    QGraphicsPixmapItem *itemCuerpo;
    QGraphicsPixmapItem *itemTronco;
    QVector<QGraphicsPixmapItem *> itemsAnimas;
    QVector<QPointF> basesAnimas;
    QGraphicsSimpleTextItem *textoInfo;
    QGraphicsSimpleTextItem *textoBarraCarga;
};

#endif
