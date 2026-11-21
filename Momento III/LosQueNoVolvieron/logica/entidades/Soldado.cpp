#include "Soldado.h"
#include "Linterna.h"
#include "Carlos.h"
#include <QtMath>

Soldado::Soldado(const QVector<QPointF> &ruta)
    : Personaje(ruta.isEmpty() ? QPointF() : ruta.first(), QSizeF(30, 30), VELOCIDAD_PATRULLA),
      ruta(ruta), indiceRuta(0), estado(EstadoSoldado::PATRULLANDO),
      linterna(new Linterna(230.0, qDegreesToRadians(21.0), qDegreesToRadians(30.0), 4.0)),
      sospecha(0.0), tiempoSinVerlo(0.0)
{
    if (ruta.size() > 1)
        indiceRuta = 1;
}

Soldado::~Soldado()
{
    delete linterna;
}

void Soldado::actualizar(double dt)
{
    if (estado == EstadoSoldado::PERSIGUIENDO)
        perseguir(dt);
    else
        patrullar(dt);

    linterna->setOrigen(posicion);
    linterna->setDireccionBase(orientacion);
    linterna->actualizar(dt);
}

void Soldado::patrullar(double dt)
{
    if (ruta.size() < 2) {
        velocidad = QPointF();
        return;
    }
    QPointF objetivo = ruta[indiceRuta];
    QPointF d = objetivo - posicion;
    double distancia = qSqrt(d.x() * d.x() + d.y() * d.y());
    if (distancia < 4.0) {
        indiceRuta = (indiceRuta + 1) % ruta.size();
        return;
    }
    // No pasarse del punto en este paso
    double factor = qMin(1.0, distancia / (velocidadBase * dt));
    mover(d, dt, factor);
}

void Soldado::perseguir(double dt)
{
    QPointF d = puntoSospechoso - posicion;
    double distancia = qSqrt(d.x() * d.x() + d.y() * d.y());
    if (distancia > 4.0)
        mover(d, dt, FACTOR_PERSECUCION);
    else
        velocidad = QPointF();
}

bool Soldado::detecta(const Carlos *carlos, double dt,
                      const std::function<bool(const QPointF &)> &bloqueaLuz)
{
    bool loVe = linterna->ilumina(carlos->getPosicion(), bloqueaLuz);

    if (loVe) {
        puntoSospechoso = carlos->getPosicion();
        tiempoSinVerlo = 0.0;
        sospecha = qMin(1.0, sospecha + dt / TIEMPO_PARA_DESCUBRIR);
        if (sospecha >= 1.0)
            estado = EstadoSoldado::PERSIGUIENDO;
    } else {
        sospecha = qMax(0.0, sospecha - dt * 0.5);
        if (estado == EstadoSoldado::PERSIGUIENDO) {
            tiempoSinVerlo += dt;
            if (tiempoSinVerlo > TIEMPO_PARA_RENDIRSE) {
                estado = EstadoSoldado::PATRULLANDO;
                sospecha = 0.0;
            }
        }
    }
    return loVe;
}

void Soldado::escucharRuido(const QPointF &origen, double intensidad)
{
    // Fase 3: estado INVESTIGANDO hacia el origen del ruido
    Q_UNUSED(origen);
    Q_UNUSED(intensidad);
}
