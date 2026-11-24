#include "Tronco.h"
#include "fisica/MovimientoOscilatorio.h"
#include <QtMath>

Tronco::Tronco(const QPointF &posicion, TipoTronco tipo, double fase)
    : Entidad(posicion, tipo == TipoTronco::TRONCO ? QSizeF(150, 30) : QSizeF(70, 40)),
      tipo(tipo),
      oleaje(new MovimientoOscilatorio(tipo == TipoTronco::TRONCO ? 6.0 : 9.0, 2.4, fase)),
      superficie(posicion.y()), velocidadRio(0.0), inclinacion(0.0) {}

Tronco::~Tronco()
{
    delete oleaje;
}

void Tronco::setAgua(double sup, double vRio)
{
    superficie = sup;
    velocidadRio = vRio;
}

void Tronco::actualizar(double dt)
{
    // Los escombros son mas livianos: van un poco mas rapido que el tronco
    double factor = (tipo == TipoTronco::TRONCO) ? 1.1 : 1.3;
    velocidad = QPointF(velocidadRio * factor, 0.0);
    posicion.setX(posicion.x() + velocidad.x() * dt);

    QPointF ola = oleaje->paso(dt);
    posicion.setY(superficie + ola.y());
    // Se inclina un poco al subir y bajar (solo para el dibujo)
    inclinacion = 0.08 * ola.y() / qMax(1.0, oleaje->getAmplitud());
}
