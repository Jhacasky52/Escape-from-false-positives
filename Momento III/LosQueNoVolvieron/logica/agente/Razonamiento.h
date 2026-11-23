#ifndef RAZONAMIENTO_H
#define RAZONAMIENTO_H

#include "Estimulo.h"
#include "Decision.h"
#include <QMap>
#include <QString>

class Aprendizaje;

// 2. RAZONAMIENTO: calcula el riesgo de las celdas vecinas a Carlos y decide.
//   riesgo = w1 * cercania de soldados + w2 * luz de linternas + w3 * exposicion
//          + w4 * peligro aprendido - w5 * avance hacia la meta
// Prioridad: OCULTARSE > ADVERTIR > CALMAR > GUIAR.
class Razonamiento
{
public:
    Razonamiento();

    Decision decidir(const Estimulo &e, const Aprendizaje *mem);

    // Ultimos riesgos calculados (para el modo depuracion)
    const QMap<QPoint, double> &getUltimoRiesgo() const { return ultimoRiesgo; }
    void setPeso(const QString &nombre, double valor) { pesos[nombre] = valor; }
    double getPeso(const QString &nombre) const { return pesos.value(nombre, 0.0); }

    // Umbrales de decision
    static constexpr double DISTANCIA_OCULTARSE = 110.0;   // px soldado-Carlos
    static constexpr double RIESGO_PELIGRO = 0.75;
    static constexpr double PANICO_CALMAR = 60.0;

private:
    double evaluarRiesgo(const CeldaPercibida &c, const Estimulo &e, const Aprendizaje *mem) const;
    double cercaniaSoldados(const QPointF &p, const Estimulo &e) const;
    double luz(const QPointF &p, const Estimulo &e) const;
    double avanceMeta(const QPointF &p, const Estimulo &e) const;

    QMap<QString, double> pesos;
    QMap<QPoint, double> ultimoRiesgo;
};

#endif
