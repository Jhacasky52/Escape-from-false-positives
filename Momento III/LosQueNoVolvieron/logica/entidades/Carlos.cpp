#include "Carlos.h"
#include "Piedra.h"
#include "niveles/Nivel.h"
#include "rasgos/ConocedorDelMonte.h"
#include "rasgos/Miedoso.h"
#include <QtMath>

Carlos::Carlos(const QPointF &posicion)
    : Personaje(posicion, QSizeF(28, 28), VELOCIDAD_CAMINAR),
      nivel(nullptr), corriendo(false), escondido(false), tiempoEscondido(0.0),
      esperaEscondite(0.0), recargaPiedra(0.0), panico(0.0), ruido(0.0),
      visibilidad(1.0), factorVelocidad(1.0), factorRuido(1.0), temblor(0.0), aire(100.0)
{
    rasgos.append(new ConocedorDelMonte());
    rasgos.append(new Miedoso());
}

Carlos::~Carlos()
{
    qDeleteAll(rasgos);
}

void Carlos::actualizar(double dt)
{
    // 1. Los rasgos ajustan los modificadores de este cuadro
    visibilidad = 1.0;
    factorVelocidad = 1.0;
    factorRuido = 1.0;
    temblor = 0.0;
    for (Rasgo *r : rasgos)
        r->aplicar(this, nivel, dt);

    // 2. Tiempos
    if (recargaPiedra > 0.0)
        recargaPiedra -= dt;
    if (escondido) {
        tiempoEscondido += dt;
        velocidad = QPointF();
        ruido = 0.0;
        return;   // escondido no se mueve
    }
    if (esperaEscondite > 0.0)
        esperaEscondite -= dt;

    // 3. Movimiento (el temblor gira un poco la direccion)
    QPointF d = direccion;
    if (!d.isNull() && temblor != 0.0) {
        double c = qCos(temblor), s = qSin(temblor);
        d = QPointF(d.x() * c - d.y() * s, d.x() * s + d.y() * c);
    }
    double factor = (corriendo ? FACTOR_CORRER : 1.0) * factorVelocidad;
    mover(d, dt, factor);

    // 4. Ruido: quieto no suena, caminando poco, corriendo mucho
    double base = 0.0;
    if (estaMoviendose())
        base = corriendo ? 1.0 : 0.25;
    ruido = qBound(0.0, base * factorRuido, 1.5);

    if (!limites.isNull()) {
        posicion.setX(qBound(limites.left(), posicion.x(), limites.right()));
        posicion.setY(qBound(limites.top(), posicion.y(), limites.bottom()));
    }
}

bool Carlos::estaMoviendose() const
{
    return !direccion.isNull() && !escondido;
}

Piedra *Carlos::lanzarPiedra(double fuerza)
{
    if (!puedeLanzar())
        return nullptr;
    recargaPiedra = TIEMPO_RECARGA_PIEDRA;
    fuerza = qBound(0.0, fuerza, 1.0);
    double v0 = 200.0 + 300.0 * fuerza;   // entre ~70 y ~420 px de alcance
    // Con panico alto la punteria tambien tiembla
    double desvio = temblor * 0.5;
    return new Piedra(posicion, orientacion + desvio, v0, qDegreesToRadians(45.0));
}

bool Carlos::esconderse()
{
    if (escondido || esperaEscondite > 0.0 || nivel == nullptr)
        return false;
    if (!nivel->esEscondite(posicion))
        return false;
    escondido = true;
    tiempoEscondido = 0.0;
    return true;
}

void Carlos::salirDelEscondite()
{
    if (!escondido)
        return;
    escondido = false;
    tiempoEscondido = 0.0;
    esperaEscondite = ESPERA_ESCONDITE;
}

void Carlos::aumentarPanico(double cantidad)
{
    panico = qMin(100.0, panico + cantidad);
}

void Carlos::calmar(double cantidad)
{
    panico = qMax(0.0, panico - cantidad);
}

double Carlos::ritmoLatidos() const
{
    return 60.0 + panico * 1.2;   // 60 lpm en calma, 180 lpm aterrorizado
}
