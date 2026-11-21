#ifndef NIVELSELVA_H
#define NIVELSELVA_H

#include "Nivel.h"
#include <QVector>
#include <QString>
#include <QPointF>

class Personaje;

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
    bool carlosEnVegetacion() const;             // fase 3: ConocedorDelMonte

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
};

#endif
