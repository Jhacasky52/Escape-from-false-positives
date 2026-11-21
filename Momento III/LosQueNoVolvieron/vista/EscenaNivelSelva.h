#ifndef ESCENANIVELSELVA_H
#define ESCENANIVELSELVA_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>

class Juego;
class NivelSelva;
class QGraphicsPixmapItem;
class QGraphicsRectItem;

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
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void construirMapa();
    void crearSpritesEntidades();

    Juego *juego;
    NivelSelva *nivel;
    double tiempoAnimacion;
    bool mapaConstruido;

    QPixmap spCarlosIdle, spCarlosWalk1, spCarlosWalk2, spCarlosRun;
    QPixmap spSoldadoWalk1, spSoldadoWalk2, spSoldadoAlerta;

    QGraphicsPixmapItem *itemCarlos;
    QVector<QGraphicsPixmapItem *> itemsSoldados;
    QVector<QGraphicsPixmapItem *> itemsConos;
    QVector<QGraphicsRectItem *> barrasSospecha;
    QGraphicsRectItem *oscuridad;
};

#endif
