#ifndef ESTIMULO_H
#define ESTIMULO_H

#include <QPointF>
#include <QPoint>
#include <QRectF>
#include <QVector>

// Cono de luz de una linterna tal como lo percibe un anima
struct ConoLuz {
    QPointF origen;
    double direccion;   // radianes
    double alcance;     // px
    double apertura;    // medio angulo, radianes
};

// Una celda cercana a Carlos que el anima puede recomendar
struct CeldaPercibida {
    QPoint celda;
    QPointF centro;
    double exposicion;   // 1 = claro abierto, 0.3 = vegetacion
    bool escondite;
};

// Todo lo que un anima percibe en un instante (salida de Percepcion).
struct Estimulo {
    QPointF posCarlos;
    QPoint celdaCarlos;
    QPointF direccionCarlos;              // hacia donde se mueve (0,0 si quieto)
    double ruidoCarlos = 0.0;
    double panicoCarlos = 0.0;
    bool carlosEscondido = false;

    QVector<QPointF> posSoldados;         // solo los que estan dentro del radio
    QVector<bool> soldadosPersiguiendo;
    QVector<ConoLuz> zonasIluminadas;     // conos de luz dentro del radio

    QVector<CeldaPercibida> celdasVecinas;
    QRectF meta;
    double tamCelda = 64.0;

    // Nivel del rio (fase 5)
    double corriente = 0.0;
    double tiempoCreciente = 0.0;
};

#endif
