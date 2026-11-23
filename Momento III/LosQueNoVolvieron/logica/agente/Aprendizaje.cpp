#include "Aprendizaje.h"
#include <QtGlobal>

Aprendizaje::Aprendizaje() : tasaObediencia(1.0) {}

void Aprendizaje::sumarPeligro(const QPoint &zona, double cantidad)
{
    double v = mapaPeligro.value(zona, 0.0) + cantidad;
    mapaPeligro[zona] = qMin(PELIGRO_MAXIMO, v);
}

void Aprendizaje::registrarCaptura(const QPoint &zona)
{
    capturas.append(zona);
    // La zona de la captura y sus vecinas quedan marcadas como peligrosas
    for (int df = -1; df <= 1; ++df)
        for (int dc = -1; dc <= 1; ++dc)
            sumarPeligro(zona + QPoint(dc, df), (dc == 0 && df == 0) ? 1.5 : 0.75);
}

void Aprendizaje::registrarAvistamiento(const QPoint &zona, double dt)
{
    // Por donde pasan mucho los soldados el peligro crece despacio (max 1)
    double actual = mapaPeligro.value(zona, 0.0);
    if (actual < 1.0)
        sumarPeligro(zona, qMin(1.0 - actual, 0.05 * dt));
}

void Aprendizaje::registrarRespuesta(const Decision &d, bool siguioGuia)
{
    historial.enqueue(d);
    if (historial.size() > TAMANO_HISTORIAL)
        historial.dequeue();
    // Promedio movil: las respuestas recientes pesan mas
    tasaObediencia = 0.85 * tasaObediencia + 0.15 * (siguioGuia ? 1.0 : 0.0);
}

double Aprendizaje::peligroAprendido(const QPoint &zona) const
{
    return qMin(1.0, mapaPeligro.value(zona, 0.0) / 1.5);
}

double Aprendizaje::insistencia() const
{
    return 1.0 - tasaObediencia;
}

void Aprendizaje::reiniciar()
{
    mapaPeligro.clear();
    capturas.clear();
    historial.clear();
    tasaObediencia = 1.0;
}
