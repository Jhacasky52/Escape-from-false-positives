#include "Linterna.h"
#include "fisica/MovimientoPendular.h"
#include <QtMath>

Linterna::Linterna(double alcance, double apertura, double anguloBarrido, double longitudPendulo)
    : alcance(alcance), apertura(apertura),
      pendulo(new MovimientoPendular(anguloBarrido, longitudPendulo)),
      direccionBase(0.0) {}

Linterna::~Linterna()
{
    delete pendulo;
}

void Linterna::actualizar(double dt)
{
    pendulo->paso(dt);
}

double Linterna::direccionActual() const
{
    return direccionBase + pendulo->anguloActual();
}

bool Linterna::ilumina(const QPointF &punto,
                       const std::function<bool(const QPointF &)> &bloqueaLuz) const
{
    QPointF d = punto - origen;
    double distancia = qSqrt(d.x() * d.x() + d.y() * d.y());
    if (distancia > alcance)
        return false;
    if (distancia < 1.0)
        return true;

    // Diferencia de angulos llevada a [-pi, pi]
    double diferencia = qAtan2(d.y(), d.x()) - direccionActual();
    while (diferencia > M_PI)  diferencia -= 2 * M_PI;
    while (diferencia < -M_PI) diferencia += 2 * M_PI;
    if (qAbs(diferencia) > apertura)
        return false;

    // Linea de vista: se revisan puntos cada 16 px entre la linterna y el punto
    if (bloqueaLuz) {
        int pasos = static_cast<int>(distancia / 16.0);
        for (int i = 1; i < pasos; ++i) {
            QPointF p = origen + d * (static_cast<double>(i) / pasos);
            if (bloqueaLuz(p))
                return false;
        }
    }
    return true;
}
