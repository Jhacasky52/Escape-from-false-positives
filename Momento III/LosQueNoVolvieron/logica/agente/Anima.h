#ifndef ANIMA_H
#define ANIMA_H

#include "Decision.h"
#include <QPointF>

class Nivel;
class Percepcion;
class Razonamiento;
class Accion;
class Aprendizaje;
class MovimientoOscilatorio;

enum class ColorAnima { BLANCA, ROJA, TENUE, RAMON };

// Anima de una victima: lucecita flotante que guia a Carlos sin hablar.
// Es un agente con cuatro modulos separados que se ejecutan en ciclo:
//   Percepcion -> Razonamiento -> Accion -> Aprendizaje
class Anima
{
public:
    // 'memoria' es compartida (la guarda Juego) y no se borra aqui
    Anima(const QPointF &posicion, Aprendizaje *memoria, int indice);
    virtual ~Anima();

    virtual void actualizar(double dt, Nivel *nivel);

    // --- Usado por Accion ---
    void mover(const QPointF &delta) { posicion += delta; }
    void setColor(ColorAnima c) { color = c; }
    void setBrillo(double b) { brillo = qBound(0.0, b, 1.0); }
    QPointF getDesplazamiento() const { return desplazamiento; }
    double getTiempo() const { return tiempo; }

    // --- Consultas (vista y pruebas) ---
    QPointF getPosicion() const { return posicion; }
    QPointF posicionDibujo() const;   // posicion + vaiven oscilatorio
    ColorAnima getColor() const { return color; }
    double getBrillo() const { return brillo; }
    const Decision &getDecision() const { return decision; }
    const Razonamiento *getRazonamiento() const { return razonamiento; }
    virtual bool esVisible() const { return true; }

protected:
    void evaluarObediencia(Nivel *nivel);

    QPointF posicion;
    ColorAnima color;
    double brillo;
    double tiempo;
    QPointF desplazamiento;   // separacion respecto a las otras animas
    MovimientoOscilatorio *vaiven;

    Percepcion *percepcion;
    Razonamiento *razonamiento;
    Accion *accion;
    Aprendizaje *aprendizaje;   // compartido, no es dueno

    Decision decision;
    double esperaDecision;      // cada cuanto piensa
    double esperaObediencia;    // cada cuanto revisa si Carlos la siguio
    QPointF carlosAlIndicar;    // donde estaba Carlos al dar la indicacion
    Decision decisionEvaluada;

    static constexpr double INTERVALO_DECISION = 0.3;
    static constexpr double INTERVALO_OBEDIENCIA = 1.5;
};

#endif
