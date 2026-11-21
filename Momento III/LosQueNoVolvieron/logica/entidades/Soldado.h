#ifndef SOLDADO_H
#define SOLDADO_H

#include "Personaje.h"
#include <QVector>
#include <functional>

class Linterna;
class Carlos;

enum class EstadoSoldado {
    PATRULLANDO,
    INVESTIGANDO,   // fase 3: va hacia el ruido de una piedra
    PERSIGUIENDO
};

// Soldado de la patrulla. Recorre su ruta con la linterna encendida.
// Si ve a Carlos durante unos instantes (la sospecha llega a 1) lo persigue;
// si lo pierde de vista un tiempo, vuelve a su ruta.
class Soldado : public Personaje
{
public:
    Soldado(const QVector<QPointF> &ruta);
    ~Soldado() override;

    void actualizar(double dt) override;

    // Revisa si ve a Carlos y actualiza la sospecha y el estado.
    // Devuelve true si en este instante lo esta viendo.
    bool detecta(const Carlos *carlos, double dt,
                 const std::function<bool(const QPointF &)> &bloqueaLuz);

    void escucharRuido(const QPointF &origen, double intensidad);   // fase 3

    EstadoSoldado getEstado() const { return estado; }
    double getSospecha() const { return sospecha; }
    const Linterna *getLinterna() const { return linterna; }
    bool estaQuieto() const { return velocidad.isNull(); }

private:
    void patrullar(double dt);
    void perseguir(double dt);

    QVector<QPointF> ruta;
    int indiceRuta;
    EstadoSoldado estado;
    Linterna *linterna;
    QPointF puntoSospechoso;   // ultima posicion conocida de Carlos
    double sospecha;           // 0..1; en 1 empieza la persecucion
    double tiempoSinVerlo;

    static constexpr double VELOCIDAD_PATRULLA = 70.0;
    static constexpr double FACTOR_PERSECUCION = 2.1;   // 147 px/s
    static constexpr double TIEMPO_PARA_DESCUBRIR = 0.7; // s dentro del cono
    static constexpr double TIEMPO_PARA_RENDIRSE = 4.0;
};

#endif
