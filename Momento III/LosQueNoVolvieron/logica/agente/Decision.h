#ifndef DECISION_H
#define DECISION_H

#include <QPointF>
#include <QPoint>

enum class TipoDecision {
    GUIAR,      // ir adelante hacia la zona mas segura que acerca a la meta
    ADVERTIR,   // peligro en la ruta: rojo y parpadeando sobre el refugio
    OCULTARSE,  // soldado muy cerca: bajar el brillo para no delatarlo
    CALMAR      // panico alto: acercarse a Carlos para calmarlo
};

// Resultado del Razonamiento
struct Decision {
    TipoDecision tipo = TipoDecision::GUIAR;
    QPointF destino;
    QPoint celdaDestino;
    double urgencia = 0.0;   // 0..1, sube con el peligro y con la insistencia
};

#endif
