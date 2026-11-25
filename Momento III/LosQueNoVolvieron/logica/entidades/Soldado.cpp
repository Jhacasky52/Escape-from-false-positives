#include "Soldado.h"
#include "Linterna.h"
#include "Carlos.h"
#include "control/Dificultad.h"
#include <QtMath>

namespace {
double largo(const QPointF &d) { return qSqrt(d.x() * d.x() + d.y() * d.y()); }
}

Soldado::Soldado(const QVector<QPointF> &ruta)
    : Personaje(ruta.isEmpty() ? QPointF() : ruta.first(), QSizeF(30, 30), VELOCIDAD_PATRULLA),
      ruta(ruta), indiceRuta(0), estado(EstadoSoldado::PATRULLANDO),
      linterna(new Linterna(230.0, qDegreesToRadians(21.0), qDegreesToRadians(30.0), 4.0)),
      volviendo(false), sospecha(0.0), tiempoSinVerlo(0.0), tiempoRevisando(0.0),
      linternaFija(false), anguloLinterna(0.0),
      tiempoDescubrir(TIEMPO_PARA_DESCUBRIR), tiempoRendirse(TIEMPO_PARA_RENDIRSE),
      olvidoSospecha(0.5), factorOido(1.0), busquedaPredictiva(false), revisaEscondites(false),
      encontroEscondido(false)
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
    switch (estado) {
    case EstadoSoldado::PATRULLANDO:  patrullar(dt); break;
    case EstadoSoldado::INVESTIGANDO: investigar(dt); break;
    case EstadoSoldado::PERSIGUIENDO: perseguir(dt); break;
    }

    linterna->setOrigen(posicion);
    linterna->setDireccionBase(linternaFija ? anguloLinterna : orientacion);
    linterna->actualizar(dt);
}

void Soldado::configurarLinterna(double alcance, double apertura, double barrido)
{
    delete linterna;
    linterna = new Linterna(alcance, apertura, barrido, 4.0);
    linterna->setOrigen(posicion);
}

// ------------------------------------------------------------ Estados

void Soldado::patrullar(double dt)
{
    // Si salio de la ruta (investigo o persiguio), primero vuelve por un camino libre
    if (volviendo) {
        if (seguirCamino(dt, 1.0))
            volviendo = false;
        return;
    }
    if (ruta.size() < 2) {
        velocidad = QPointF();
        return;
    }
    QPointF d = ruta[indiceRuta] - posicion;
    double distancia = largo(d);
    if (distancia < 4.0) {
        indiceRuta = (indiceRuta + 1) % ruta.size();
        return;
    }
    mover(d, dt, qMin(1.0, distancia / (velocidadBase * dt)));   // no pasarse del punto
}

void Soldado::investigar(double dt)
{
    if (!seguirCamino(dt, FACTOR_INVESTIGAR))
        return;
    // Llego al lugar del ruido: gira mirando alrededor con la linterna
    velocidad = QPointF();
    tiempoRevisando += dt;
    orientacion += 1.6 * dt;
    if (tiempoRevisando >= TIEMPO_REVISANDO) {
        estado = EstadoSoldado::PATRULLANDO;
        volverALaRuta();
    }
}

void Soldado::perseguir(double dt)
{
    QPointF d = puntoSospechoso - posicion;
    if (tiempoSinVerlo <= 0.0) {
        // Lo esta viendo: va directo hacia el
        camino.clear();
        if (largo(d) > 4.0)
            mover(d, dt, FACTOR_PERSECUCION);
        else
            velocidad = QPointF();
    } else {
        // Lo perdio de vista: va al ultimo lugar donde lo vio rodeando los arboles
        if (camino.isEmpty())
            irA(puntoSospechoso);
        seguirCamino(dt, FACTOR_PERSECUCION);
    }
}

// ------------------------------------------------------------ Percepcion

bool Soldado::detecta(const Carlos *carlos, double dt,
                      const std::function<bool(const QPointF &)> &bloqueaLuz)
{
    bool enLaLuz = linterna->ilumina(carlos->getPosicion(), bloqueaLuz);
    double visibilidad = carlos->getVisibilidad();   // la vegetacion y los escondites la bajan

    // Dificil: al revisar la zona de un ruido mira dentro de los escondites cercanos
    bool revisando = estado == EstadoSoldado::INVESTIGANDO && tiempoRevisando > 0.0;
    if (revisaEscondites && revisando && carlos->estaEscondido()
        && largo(carlos->getPosicion() - posicion) < RADIO_REVISION) {
        enLaLuz = true;
        visibilidad = 0.6;
        encontroEscondido = true;
    }
    bool loVe = enLaLuz && visibilidad > 0.0;

    if (loVe) {
        sospecha = qMin(1.0, sospecha + visibilidad * dt / tiempoDescubrir);
        if (!carlos->getVelocidad().isNull())
            ultimaVelocidadCarlos = carlos->getVelocidad();
        if (estado == EstadoSoldado::PERSIGUIENDO || sospecha >= 1.0) {
            if (estado != EstadoSoldado::PERSIGUIENDO)
                camino.clear();
            estado = EstadoSoldado::PERSIGUIENDO;
            puntoSospechoso = carlos->getPosicion();
            tiempoSinVerlo = 0.0;
        }
    } else {
        sospecha = qMax(0.0, sospecha - dt * olvidoSospecha);
        if (!carlos->estaEscondido())
            encontroEscondido = false;
        if (estado == EstadoSoldado::PERSIGUIENDO) {
            tiempoSinVerlo += dt;
            if (tiempoSinVerlo > tiempoRendirse) {
                // Se rinde, pero revisa la zona donde lo vio por ultima vez.
                // Con busqueda predictiva revisa hacia donde iba Carlos.
                estado = EstadoSoldado::INVESTIGANDO;
                sospecha = 0.0;
                tiempoRevisando = 0.0;
                if (busquedaPredictiva)
                    puntoSospechoso += ultimaVelocidadCarlos * SEGUNDOS_PREDICCION;
                irA(puntoSospechoso);
            }
        }
    }
    return loVe;
}

void Soldado::configurar(const Dificultad &d)
{
    tiempoDescubrir = d.tiempoParaDescubrir;
    tiempoRendirse = d.tiempoParaRendirse;
    olvidoSospecha = d.olvidoSospecha;
    factorOido = d.factorOido;
    busquedaPredictiva = d.busquedaPredictiva;
    revisaEscondites = d.revisaEscondites;
}

void Soldado::recibirAviso(const QPointF &punto)
{
    if (estado == EstadoSoldado::PERSIGUIENDO)
        return;
    estado = EstadoSoldado::INVESTIGANDO;
    puntoSospechoso = punto;
    tiempoRevisando = 0.0;
    volviendo = false;
    irA(punto);
}

void Soldado::escucharRuido(const QPointF &origen, double intensidad)
{
    if (estado == EstadoSoldado::PERSIGUIENDO)
        return;   // si ya lo persigue, no se distrae
    if (largo(origen - posicion) > RADIO_OIDO * factorOido * intensidad)
        return;   // muy lejos para oirlo
    estado = EstadoSoldado::INVESTIGANDO;
    puntoSospechoso = origen;
    tiempoRevisando = 0.0;
    volviendo = false;
    irA(origen);
}

// ------------------------------------------------------------ Caminos

void Soldado::irA(const QPointF &destino)
{
    camino.clear();
    if (planificador)
        camino = planificador(posicion, destino);
    if (camino.isEmpty())
        camino.append(destino);   // sin mapa: en linea recta
}

bool Soldado::seguirCamino(double dt, double factor)
{
    while (!camino.isEmpty()) {
        QPointF d = camino.first() - posicion;
        double distancia = largo(d);
        if (distancia < 6.0) {
            camino.removeFirst();
            continue;
        }
        double paso = velocidadBase * factor * dt;
        mover(d, dt, factor * qMin(1.0, distancia / paso));
        return false;
    }
    return true;
}

void Soldado::volverALaRuta()
{
    if (ruta.isEmpty())
        return;
    // Retoma la ruta en el punto mas cercano
    int mejor = 0;
    for (int i = 1; i < ruta.size(); ++i)
        if (largo(ruta[i] - posicion) < largo(ruta[mejor] - posicion))
            mejor = i;
    indiceRuta = mejor;
    irA(ruta[mejor]);
    volviendo = true;
}
