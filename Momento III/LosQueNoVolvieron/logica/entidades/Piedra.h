#ifndef PIEDRA_H
#define PIEDRA_H

#include "Entidad.h"

class MovimientoParabolico;

// Piedra que lanza Carlos para distraer a los soldados.
// Usa MovimientoParabolico: la distancia recorrida se aplica en la direccion
// del lanzamiento (vista cenital) y la altura z se dibuja como tamano + sombra.
// Al caer hace ruido: el nivel avisa a los soldados cercanos.
class Piedra : public Entidad
{
public:
    // direccion en radianes; v0 en px/s; angulo de elevacion en radianes
    Piedra(const QPointF &origen, double direccion, double v0, double elevacion);
    ~Piedra() override;

    void actualizar(double dt) override;

    bool cayo() const;               // ya toco el suelo
    void detener();                  // choco contra un arbol: cae ahi mismo
    double getAltura() const { return altura; }
    QPointF getOrigen() const { return origen; }
    bool estaActiva() const { return activa; }
    void desactivar() { activa = false; }   // el nivel ya proceso su ruido

    // Intensidad del ruido al caer (0..1), mayor si cae desde mas alto/lejos
    double intensidadRuido() const { return intensidad; }

private:
    QPointF origen;
    double direccion;
    double altura;
    bool activa;
    bool detenida;
    double intensidad;
    MovimientoParabolico *fisica;
};

#endif
