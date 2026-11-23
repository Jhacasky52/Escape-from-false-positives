#ifndef APRENDIZAJE_H
#define APRENDIZAJE_H

#include "Decision.h"
#include <QMap>
#include <QPoint>
#include <QVector>
#include <QQueue>

// QMap necesita poder ordenar las llaves: se ordenan las celdas por fila y columna
inline bool operator<(const QPoint &a, const QPoint &b)
{
    return a.y() < b.y() || (a.y() == b.y() && a.x() < b.x());
}

// 4. APRENDIZAJE: memoria compartida por todas las animas.
// Vive en Juego, asi que se conserva aunque el nivel se reinicie.
//  - mapaPeligro: cuanto peligro hay en cada celda (capturas y avistamientos).
//  - tasaObediencia: si Carlos sigue o no las indicaciones -> insistencia.
class Aprendizaje
{
public:
    Aprendizaje();

    void registrarCaptura(const QPoint &zona);
    void registrarAvistamiento(const QPoint &zona, double dt);
    void registrarRespuesta(const Decision &d, bool siguioGuia);

    double peligroAprendido(const QPoint &zona) const;   // 0..1
    double insistencia() const;                          // 0..1 (1 = Carlos las ignora)
    double getTasaObediencia() const { return tasaObediencia; }
    int numeroCapturas() const { return capturas.size(); }
    const QMap<QPoint, double> &getMapaPeligro() const { return mapaPeligro; }

    // Para guardar partida (fase 6)
    void setMapaPeligro(const QMap<QPoint, double> &m) { mapaPeligro = m; }
    void reiniciar();

private:
    void sumarPeligro(const QPoint &zona, double cantidad);

    QMap<QPoint, double> mapaPeligro;
    QVector<QPoint> capturas;
    QQueue<Decision> historial;   // ultimas decisiones evaluadas
    double tasaObediencia;        // 0..1

    static constexpr int TAMANO_HISTORIAL = 20;
    static constexpr double PELIGRO_MAXIMO = 3.0;
};

#endif
