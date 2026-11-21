#ifndef LINTERNA_H
#define LINTERNA_H

#include <QPointF>
#include <functional>

class MovimientoPendular;

// Linterna de un soldado. Su cono de luz apunta hacia donde mira el soldado
// (direccionBase) mas el barrido del pendulo.
class Linterna
{
public:
    // alcance en px, apertura = MEDIO angulo del cono en radianes
    Linterna(double alcance, double apertura, double anguloBarrido, double longitudPendulo);
    ~Linterna();

    void actualizar(double dt);

    void setOrigen(const QPointF &o) { origen = o; }
    void setDireccionBase(double radianes) { direccionBase = radianes; }

    // Direccion real del cono (radianes, 0 = derecha, pi/2 = abajo)
    double direccionActual() const;
    QPointF getOrigen() const { return origen; }
    double getAlcance() const { return alcance; }
    double getApertura() const { return apertura; }

    // true si el punto esta dentro del cono. 'bloqueaLuz' (opcional) dice si
    // un punto del camino esta tapado (por ejemplo por un arbol).
    bool ilumina(const QPointF &punto,
                 const std::function<bool(const QPointF &)> &bloqueaLuz = nullptr) const;

private:
    double alcance;
    double apertura;
    MovimientoPendular *pendulo;
    QPointF origen;
    double direccionBase;
};

#endif
