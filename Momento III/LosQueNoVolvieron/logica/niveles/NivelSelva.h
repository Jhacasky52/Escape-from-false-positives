#ifndef NIVELSELVA_H
#define NIVELSELVA_H

#include "Nivel.h"
#include <QVector>
#include <QString>
#include <QPointF>
#include <QList>

class Personaje;
class Piedra;

// Tipos de celda del mapa (ver recursos/mapas/selva.txt)
enum class Celda {
    SUELO,
    TROCHA,
    ARBOL,      // bloquea el paso y la luz
    MATORRAL,   // se puede pisar, esconde (fase 3)
    ESCONDITE,  // tronco hueco (fase 3)
    RIO         // meta
};

// Nivel 1: la selva, vista cenital con scroll.
class NivelSelva : public Nivel
{
public:
    explicit NivelSelva(const QString &rutaMapa = ":/mapas/selva.txt");
    ~NivelSelva() override;

    void cargar() override;
    void actualizar(double dt) override;

    // --- Consultas del mapa ---
    static constexpr int TAM_CELDA = 64;
    int getColumnas() const { return columnas; }
    int getFilas() const { return filas; }
    Celda celdaEn(int columna, int fila) const;
    Celda celdaEnPunto(const QPointF &p) const;
    bool esSolido(const QRectF &caja) const;     // choca con algun arbol
    bool bloqueaLuz(const QPointF &p) const;
    bool carlosEnVegetacion() const;

    // Terreno para los rasgos de Carlos
    bool esVegetacion(const QPointF &p) const override;
    bool esTrocha(const QPointF &p) const override;
    bool esEscondite(const QPointF &p) const override;

    // Carlos lanza una piedra con fuerza 0..1 (false si aun no puede)
    bool lanzarPiedra(double fuerza);
    const QList<Piedra *> &getPiedras() const { return piedras; }

    // Camino libre de arboles entre dos puntos (busqueda en anchura sobre la cuadricula)
    QVector<QPointF> buscarCamino(const QPointF &desde, const QPointF &hasta) const;
    bool esTransitable(int columna, int fila) const;

    bool algunSoldadoPersigue() const;

private:
    void leerMapa();
    void moverConColision(Personaje *p, const QPointF &anterior);
    QPointF centroCelda(int columna, int fila) const;

    QString rutaMapa;
    int columnas;
    int filas;
    QVector<Celda> celdas;
    QPointF inicioCarlos;
    QVector<QVector<QPointF>> rutasSoldados;
    QList<Piedra *> piedras;    // memoria dinamica: el nivel es el dueno
    double temporizadorPasos;   // cada cuanto los pasos de Carlos hacen ruido
};

#endif
