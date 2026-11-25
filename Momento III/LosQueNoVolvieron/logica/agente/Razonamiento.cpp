#include "Razonamiento.h"
#include "Aprendizaje.h"
#include <QtMath>
#include <limits>

namespace {
double largo(const QPointF &d) { return qSqrt(d.x() * d.x() + d.y() * d.y()); }
}

Razonamiento::Razonamiento()
{
    pesos["cercania"] = 1.0;    // w1
    pesos["luz"] = 1.2;         // w2
    pesos["exposicion"] = 0.25; // w3
    pesos["aprendido"] = 0.9;   // w4
    pesos["meta"] = 0.6;        // w5
}

double Razonamiento::cercaniaSoldados(const QPointF &p, const Estimulo &e) const
{
    double maximo = 0.0;
    for (int i = 0; i < e.posSoldados.size(); ++i) {
        double d = largo(e.posSoldados[i] - p);
        double alcance = e.soldadosPersiguiendo.value(i) ? 420.0 : 300.0;
        maximo = qMax(maximo, qMax(0.0, 1.0 - d / alcance));
    }
    return maximo;
}

double Razonamiento::luz(const QPointF &p, const Estimulo &e) const
{
    double total = 0.0;
    for (const ConoLuz &c : e.zonasIluminadas) {
        QPointF d = p - c.origen;
        double distancia = largo(d);
        if (distancia > c.alcance * 1.15)   // un margen: el cono se mueve
            continue;
        double diferencia = qAtan2(d.y(), d.x()) - c.direccion;
        while (diferencia > M_PI)  diferencia -= 2 * M_PI;
        while (diferencia < -M_PI) diferencia += 2 * M_PI;
        // El pendulo barre ~30 grados: lo que esta cerca del cono tambien es riesgoso
        double margen = c.apertura + qDegreesToRadians(25.0);
        if (qAbs(diferencia) <= c.apertura)
            total = qMax(total, 1.0);
        else if (qAbs(diferencia) <= margen)
            total = qMax(total, 0.5);
    }
    return total;
}

double Razonamiento::avanceMeta(const QPointF &p, const Estimulo &e) const
{
    // Distancia horizontal/vertical al rectangulo de la meta
    auto distanciaMeta = [&e](const QPointF &q) {
        double dx = qMax(0.0, qMax(e.meta.left() - q.x(), q.x() - e.meta.right()));
        double dy = qMax(0.0, qMax(e.meta.top() - q.y(), q.y() - e.meta.bottom()));
        return qSqrt(dx * dx + dy * dy);
    };
    // Positivo si la celda acerca a Carlos a la meta (normalizado a ~1 por 3 celdas)
    return (distanciaMeta(e.posCarlos) - distanciaMeta(p)) / (3.0 * e.tamCelda);
}

double Razonamiento::evaluarRiesgo(const CeldaPercibida &c, const Estimulo &e,
                                   const Aprendizaje *mem) const
{
    double aprendido = mem != nullptr ? mem->peligroAprendido(c.celda) : 0.0;
    return pesos.value("cercania") * cercaniaSoldados(c.centro, e)
         + pesos.value("luz") * luz(c.centro, e)
         + pesos.value("exposicion") * c.exposicion
         + pesos.value("aprendido") * aprendido
         - pesos.value("meta") * avanceMeta(c.centro, e);
}

Decision Razonamiento::decidir(const Estimulo &e, const Aprendizaje *mem)
{
    Decision d;
    ultimoRiesgo.clear();
    double insistencia = mem != nullptr ? mem->insistencia() : 0.0;

    // Evaluar todas las celdas vecinas
    const CeldaPercibida *mejor = nullptr, *mejorEscondite = nullptr;
    double riesgoMejor = std::numeric_limits<double>::max();
    double riesgoEscondite = std::numeric_limits<double>::max();
    double riesgoActual = 0.0, riesgoAdelante = 0.0;
    QPoint celdaAdelante = e.celdaCarlos;
    if (!e.direccionCarlos.isNull()) {
        // La celda a la que Carlos se dirige
        QPointF adelante = e.posCarlos + e.direccionCarlos / largo(e.direccionCarlos) * e.tamCelda;
        celdaAdelante = QPoint(qFloor(adelante.x() / e.tamCelda), qFloor(adelante.y() / e.tamCelda));
    }

    for (const CeldaPercibida &c : e.celdasVecinas) {
        double r = evaluarRiesgo(c, e, mem);
        ultimoRiesgo[c.celda] = r;
        if (c.celda == e.celdaCarlos)
            riesgoActual = r;
        if (c.celda == celdaAdelante)
            riesgoAdelante = r;
        if (r < riesgoMejor) {
            riesgoMejor = r;
            mejor = &c;
        }
        if (c.escondite && r < riesgoEscondite) {
            riesgoEscondite = r;
            mejorEscondite = &c;
        }
    }
    if (mejor == nullptr) {   // sin celdas libres: quedarse con Carlos
        d.tipo = TipoDecision::CALMAR;
        d.destino = e.posCarlos;
        d.celdaDestino = e.celdaCarlos;
        return d;
    }

    double soldadoMasCerca = std::numeric_limits<double>::max();
    for (const QPointF &s : e.posSoldados)
        soldadoMasCerca = qMin(soldadoMasCerca, largo(s - e.posCarlos));

    // Prioridad 1: soldado encima -> apagarse para no delatarlo
    if (soldadoMasCerca < DISTANCIA_OCULTARSE) {
        d.tipo = TipoDecision::OCULTARSE;
        d.destino = e.posCarlos;
        d.celdaDestino = e.celdaCarlos;
        d.urgencia = 1.0;
        return d;
    }
    // Prioridad 2: peligro donde esta o hacia donde va -> advertir y senalar refugio
    double peligro = qMax(riesgoActual, riesgoAdelante);
    if (peligro > umbralRiesgo) {
        const CeldaPercibida *refugio = (mejorEscondite != nullptr && riesgoEscondite < peligro)
                                        ? mejorEscondite : mejor;
        d.tipo = TipoDecision::ADVERTIR;
        d.destino = refugio->centro;
        d.celdaDestino = refugio->celda;
        d.urgencia = qMin(1.0, peligro / 2.0 + insistencia * 0.5);
        return d;
    }
    // Prioridad 2b (rio): se acerca la creciente y Carlos esta en el agua ->
    // advertir y senalar la roca alta u orilla seca mas cercana
    if (e.corriente != 0.0 && e.hayRefugio && e.tiempoCreciente >= 0.0
        && e.tiempoCreciente < AVISO_CRECIENTE) {
        d.tipo = TipoDecision::ADVERTIR;
        d.destino = e.refugio;
        d.celdaDestino = QPoint(qFloor(e.refugio.x() / e.tamCelda), qFloor(e.refugio.y() / e.tamCelda));
        d.urgencia = qMin(1.0, 1.0 - e.tiempoCreciente / AVISO_CRECIENTE + insistencia * 0.5);
        return d;
    }
    // Prioridad 3: panico alto -> ir a calmarlo
    if (e.panicoCarlos > PANICO_CALMAR) {
        d.tipo = TipoDecision::CALMAR;
        d.destino = e.posCarlos;
        d.celdaDestino = e.celdaCarlos;
        d.urgencia = e.panicoCarlos / 100.0;
        return d;
    }
    // Si no pasa nada: guiar hacia la zona de menor riesgo
    d.tipo = TipoDecision::GUIAR;
    d.destino = mejor->centro;
    d.celdaDestino = mejor->celda;
    d.urgencia = insistencia;
    return d;
}
