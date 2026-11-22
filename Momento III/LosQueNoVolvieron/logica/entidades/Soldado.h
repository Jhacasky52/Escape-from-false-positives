#ifndef SOLDADO_H
#define SOLDADO_H

#include "Personaje.h"
#include <QVector>
#include <functional>

class Linterna;
class Carlos;

enum class EstadoSoldado {
    PATRULLANDO,    // recorre su ruta
    INVESTIGANDO,   // va hacia un ruido (piedra o pasos) y revisa la zona
    PERSIGUIENDO    // vio a Carlos y va tras el
};

// Calcula un camino (lista de puntos) entre dos posiciones esquivando obstaculos.
// Lo entrega el nivel, que es quien conoce el mapa.
using Planificador = std::function<QVector<QPointF>(const QPointF &, const QPointF &)>;

// Soldado de la patrulla (maquina de estados de tres estados).
class Soldado : public Personaje
{
public:
    explicit Soldado(const QVector<QPointF> &ruta);
    ~Soldado() override;

    void actualizar(double dt) override;

    // Revisa si ve a Carlos y actualiza la sospecha y el estado.
    bool detecta(const Carlos *carlos, double dt,
                 const std::function<bool(const QPointF &)> &bloqueaLuz);

    // Un ruido de intensidad 0..1 se oye hasta RADIO_OIDO * intensidad px
    void escucharRuido(const QPointF &origen, double intensidad);

    void setPlanificador(const Planificador &p) { planificador = p; }

    EstadoSoldado getEstado() const { return estado; }
    double getSospecha() const { return sospecha; }
    const Linterna *getLinterna() const { return linterna; }
    QPointF getPuntoSospechoso() const { return puntoSospechoso; }

    static constexpr double RADIO_OIDO = 380.0;

private:
    void patrullar(double dt);
    void investigar(double dt);
    void perseguir(double dt);
    void irA(const QPointF &destino);          // calcula el camino con el planificador
    bool seguirCamino(double dt, double factor); // true al llegar al final
    void volverALaRuta();

    QVector<QPointF> ruta;
    int indiceRuta;
    EstadoSoldado estado;
    Linterna *linterna;
    Planificador planificador;
    QVector<QPointF> camino;   // puntos pendientes cuando sale de su ruta
    bool volviendo;            // regresa a la ruta despues de investigar/perseguir
    QPointF puntoSospechoso;
    double sospecha;           // 0..1; en 1 empieza la persecucion
    double tiempoSinVerlo;
    double tiempoRevisando;    // cuanto lleva mirando alrededor del ruido

    static constexpr double VELOCIDAD_PATRULLA = 70.0;
    static constexpr double FACTOR_INVESTIGAR = 1.4;
    static constexpr double FACTOR_PERSECUCION = 2.1;    // 147 px/s
    static constexpr double TIEMPO_PARA_DESCUBRIR = 0.7; // s a plena vista
    static constexpr double TIEMPO_PARA_RENDIRSE = 4.0;
    static constexpr double TIEMPO_REVISANDO = 3.0;
};

#endif
