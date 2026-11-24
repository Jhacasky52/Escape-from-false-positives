#ifndef ESCENANIVELRIO_H
#define ESCENANIVELRIO_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QVector>

class Juego;
class NivelRio;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsPolygonItem;
class QGraphicsSimpleTextItem;
#ifdef HAY_MULTIMEDIA
class QSoundEffect;
#endif

// Capa grafica del nivel 2 (el rio, visto de lado). Lee el estado de NivelRio
// y mueve los sprites; la camara sigue a Carlos en horizontal.
class EscenaNivelRio : public EscenaJuego
{
    Q_OBJECT
public:
    EscenaNivelRio(Juego *juego, NivelRio *nivel, QObject *parent = nullptr);

    // Vuelve a crear los sprites (al entrar al nivel o al reiniciarlo)
    void reconstruir();

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    void crearApoyos();
    void crearSpritesEntidades();
    void actualizarTroncos();
    void actualizarAnimas();
    void actualizarLatidos(double dt);
    void dibujarDepuracion(QPainter *painter);
    static QPixmap sinAgua(const QPixmap &p);   // quita el agua pintada en el sprite

    Juego *juego;
    NivelRio *nivel;
    double tiempoAnimacion;
    double faseLatido;
    double pulso;
    bool modoDepuracion;

    QPixmap spFondo[3];
    QPixmap spCarlosNada1, spCarlosNada2, spCarlosSumergido, spCarlosArrastrado;
    QPixmap spCarlosPie1, spCarlosPie2;
    QPixmap spSoldado;
    QPixmap spTronco, spEscombros, spRoca, spBanco;
    QPixmap spAnimaBlanca, spAnimaRoja, spAnimaTenue, spAnimaRamon;

    QGraphicsPixmapItem *itemCarlos;
    QVector<QGraphicsPixmapItem *> itemsSoldados;
    QVector<QGraphicsPolygonItem *> itemsConos;
    QVector<QGraphicsRectItem *> barrasSospecha;
    QVector<QGraphicsSimpleTextItem *> signosSoldados;
    QVector<QGraphicsPixmapItem *> itemsTroncos;
    QVector<QGraphicsPixmapItem *> itemsAnimas;
    QVector<QGraphicsItem *> itemsApoyos;
    QGraphicsRectItem *agua;
    QGraphicsRectItem *lineaAgua;
#ifdef HAY_MULTIMEDIA
    QSoundEffect *sonidoLatido;
#endif
};

#endif
