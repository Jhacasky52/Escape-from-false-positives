#ifndef ESCENANIVELSELVA_H
#define ESCENANIVELSELVA_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>

class Juego;
class NivelSelva;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;
#ifdef HAY_MULTIMEDIA
class QSoundEffect;
#endif

// Capa grafica del nivel 1. Lee el estado de NivelSelva en cada cuadro y
// mueve los sprites; la camara sigue a Carlos. No contiene reglas del juego.
class EscenaNivelSelva : public EscenaJuego
{
    Q_OBJECT
public:
    EscenaNivelSelva(Juego *juego, NivelSelva *nivel, QObject *parent = nullptr);

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void teclaSoltada(int tecla) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void construirMapa();
    void crearSpritesEntidades();
    void actualizarPiedras();
    void actualizarAnimas();
    void dibujarDepuracion(QPainter *painter);
    void actualizarLatidos(double dt);

    Juego *juego;
    NivelSelva *nivel;
    double tiempoAnimacion;
    bool mapaConstruido;
    bool cargando;          // Espacio presionado
    double carga;           // 0..1
    double faseLatido;      // 0..1 dentro de un latido
    double pulso;           // brillo del corazon en el HUD (1 al latir, baja a 0)

    QPixmap spCarlosIdle, spCarlosWalk1, spCarlosWalk2, spCarlosRun, spCarlosThrow, spCarlosEscondido;
    QPixmap spPiedra[3];
    QPixmap spAnimaBlanca, spAnimaRoja, spAnimaTenue, spAnimaRamon;
    bool modoDepuracion;    // F1: muestra lo que piensan las animas
    QPixmap spSoldadoWalk1, spSoldadoWalk2, spSoldadoAlerta;

    QGraphicsPixmapItem *itemCarlos;
    QVector<QGraphicsPixmapItem *> itemsSoldados;
    QVector<QGraphicsPixmapItem *> itemsConos;
    QVector<QGraphicsRectItem *> barrasSospecha;
    QVector<QGraphicsSimpleTextItem *> signosSoldados;   // "?" al investigar
    QVector<QGraphicsPixmapItem *> itemsPiedras;
    QVector<QGraphicsPixmapItem *> itemsAnimas;
    QVector<QGraphicsEllipseItem *> sombrasPiedras;
#ifdef HAY_MULTIMEDIA
    QSoundEffect *sonidoLatido;
#endif
    QGraphicsRectItem *oscuridad;
};

#endif
