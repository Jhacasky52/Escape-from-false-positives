#ifndef NIVEL_H
#define NIVEL_H

#include <QRectF>
#include <QList>
#include <QPointF>
#include <QPoint>
#include "control/Dificultad.h"

class Carlos;
class Soldado;
class Anima;
class Aprendizaje;

// Clase abstracta con lo comun a todos los niveles.
// NivelSelva (fase 2) y NivelRio (fase 5) heredan de aqui.
// Las animas comparten una memoria (Aprendizaje) que guarda Juego.
class Nivel
{
public:
    Nivel();
    virtual ~Nivel();

    virtual void cargar() = 0;
    virtual void actualizar(double dt) = 0;

    // Consultas del terreno (cada nivel las responde a su manera)
    virtual bool esVegetacion(const QPointF &p) const { Q_UNUSED(p); return false; }
    virtual bool esTrocha(const QPointF &p) const { Q_UNUSED(p); return false; }
    virtual bool esEscondite(const QPointF &p) const { Q_UNUSED(p); return false; }
    virtual bool esTransitable(const QPointF &p) const { return limites.contains(p); }

    // Nivel del rio (fase 5). En la selva no hay corriente ni creciente.
    virtual double corrienteEn(const QPointF &p) const { Q_UNUSED(p); return 0.0; }
    virtual double tiempoParaCreciente() const { return -1.0; }   // -1 = no aplica
    // Refugio seco mas cercano (roca alta u orilla); false si el nivel no tiene
    virtual bool refugioCercano(const QPointF &p, QPointF &refugio) const
    { Q_UNUSED(p); Q_UNUSED(refugio); return false; }

    // Cuadricula comun (la usan las animas para razonar por zonas)
    virtual double tamCelda() const { return 64.0; }
    QPoint celdaDe(const QPointF &p) const;
    QPointF centroDe(const QPoint &celda) const;

    // Un ruido en 'origen' llega a los soldados que esten lo bastante cerca
    void hacerRuido(const QPointF &origen, double intensidad);

    virtual bool haGanado() const;
    virtual bool haPerdido() const;
    virtual void reiniciar();

    Carlos *getCarlos() const { return carlos; }
    const QList<Soldado *> &getSoldados() const { return soldados; }
    const QList<Anima *> &getAnimas() const { return animas; }
    void setMemoria(Aprendizaje *m) { memoria = m; }
    // Se aplica al cargar el nivel (por ahora solo la usa la selva)
    void setDificultad(const Dificultad &d) { dificultad = d; }
    const Dificultad &getDificultad() const { return dificultad; }
    Aprendizaje *getMemoria() const { return memoria; }
    QRectF getMeta() const { return meta; }
    QRectF getLimites() const { return limites; }

protected:
    void liberar();   // borra todas las entidades creadas con new
    void actualizarAnimas(double dt);

    Carlos *carlos;
    QList<Soldado *> soldados;   // memoria dinamica: el nivel es el dueno
    QList<Anima *> animas;       // idem
    Aprendizaje *memoria;        // compartida, la guarda Juego (no es dueno)
    QRectF meta;
    QRectF limites;
    bool capturado;
    Dificultad dificultad;
};

#endif
