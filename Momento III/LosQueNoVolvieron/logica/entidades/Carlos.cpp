#include "Carlos.h"
#include "Piedra.h"
#include "fisica/MovimientoFluido.h"
#include "niveles/Nivel.h"
#include "rasgos/ConocedorDelMonte.h"
#include "rasgos/Miedoso.h"
#include <QtMath>

Carlos::Carlos(const QPointF &posicion)
    : Personaje(posicion, QSizeF(28, 28), VELOCIDAD_CAMINAR),
      nivel(nullptr), corriendo(false), escondido(false), tiempoEscondido(0.0),
      esperaEscondite(0.0), recargaPiedra(0.0), panico(0.0), ruido(0.0),
      visibilidad(1.0), factorVelocidad(1.0), factorRuido(1.0), temblor(0.0), aire(100.0),
      vistaLateral(false), enAgua(false), fluido(nullptr), superficie(0.0), velocidadRio(0.0)
{
    rasgos.append(new ConocedorDelMonte());
    rasgos.append(new Miedoso());
}

Carlos::~Carlos()
{
    qDeleteAll(rasgos);
    delete fluido;
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

    if (enAgua) {
        nadar(dt);
        return;
    }
    aire = qMin(100.0, aire + RECUPERA_AIRE * dt);

    // 3. Movimiento (el temblor gira un poco la direccion)
    QPointF d = direccion;
    if (vistaLateral)
        d.setY(0.0);   // vista de lado: en tierra solo camina a izquierda o derecha
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
    if (vistaLateral && !enAgua)
        return direccion.x() != 0.0 && !escondido;
    return !direccion.isNull() && !escondido;
}

// ------------------------------------------------------------ Rio

void Carlos::entrarAlAgua()
{
    if (enAgua)
        return;
    if (fluido == nullptr)
        fluido = new MovimientoFluido(MASA, K_CORRIENTE, B_RESISTENCIA, velocidadRio);
    fluido->reiniciar();
    fluido->setVelocidadRio(velocidadRio);
    fluido->setVelocidad(QPointF(velocidad.x(), 60.0));   // cae al agua con algo de impulso
    desplazamientoAgua = QPointF();
    enAgua = true;
    escondido = false;
}

void Carlos::salirDelAgua(const QPointF &pie)
{
    enAgua = false;
    posicion = pie;
    velocidad = QPointF();
}

void Carlos::setAgua(double sup, double vRio)
{
    superficie = sup;
    velocidadRio = vRio;
}

void Carlos::empujar(const QPointF &v)
{
    if (enAgua && fluido != nullptr) {
        fluido->setVelocidad(v);
        velocidad = v;
    }
}

void Carlos::nadar(double dt)
{
    // Direccion con el temblor del panico
    QPointF d = direccion;
    if (!d.isNull() && temblor != 0.0) {
        double c = qCos(temblor), s = qSin(temblor);
        d = QPointF(d.x() * c - d.y() * s, d.x() * s + d.y() * c);
    }

    // Fuerzas que hace Carlos: brazadas en x; en y la flotacion lo lleva a la
    // superficie y al bucear empuja hacia el fondo
    double prof = profundidad();
    double fx = d.x() * FUERZA_NADO * factorVelocidad;
    double fy = -K_FLOTACION * prof;
    if (direccion.y() > 0.0)
        fy += FUERZA_BUCEO;
    else if (direccion.y() < 0.0 && prof > 0.0)
        fy -= FUERZA_BUCEO * 0.5;   // sube mas rapido

    fluido->setVelocidadRio(velocidadRio);
    fluido->setFuerzaExterna(QPointF(fx, fy));
    QPointF nuevo = fluido->paso(dt);
    posicion += nuevo - desplazamientoAgua;
    desplazamientoAgua = nuevo;
    velocidad = fluido->getVelocidad();
    if (fx > 0.0)
        orientacion = 0.0;
    else if (fx < 0.0)
        orientacion = M_PI;

    // Aire, ruido (chapoteo) y visibilidad
    prof = profundidad();
    if (prof > PROF_OCULTO) {
        aire = qMax(0.0, aire - GASTO_AIRE * dt);
        ruido = 0.0;
        visibilidad = 0.0;   // bajo el agua las linternas no lo ven
    } else {
        aire = qMin(100.0, aire + RECUPERA_AIRE * dt);
        ruido = qBound(0.0, (d.isNull() ? 0.0 : 0.35) * factorRuido, 1.5);
        // Entre la superficie y PROF_OCULTO se ve cada vez menos
        visibilidad *= 0.8 * (1.0 - qBound(0.0, prof / PROF_OCULTO, 1.0)) + 0.1;
    }

    if (!limites.isNull()) {
        posicion.setX(qBound(limites.left(), posicion.x(), limites.right()));
        posicion.setY(qBound(limites.top(), posicion.y(), limites.bottom()));
    }
}

Piedra *Carlos::lanzarPiedra(double fuerza)
{
    if (!puedeLanzar() || enAgua)
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
    if (enAgua)
        return false;   // en el agua se esconde buceando
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
