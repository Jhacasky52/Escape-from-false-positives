// Pruebas automaticas de las cuatro fisicas (sin graficos).
// En Qt Creator: elegir el objetivo "pruebas_fisica" y ejecutarlo.
#include "fisica/MovimientoParabolico.h"
#include "fisica/MovimientoPendular.h"
#include "fisica/MovimientoFluido.h"
#include "fisica/MovimientoOscilatorio.h"

#include <QtMath>
#include <cstdio>

static int fallos = 0;

static void verificar(bool condicion, const char *descripcion)
{
    std::printf("[%s] %s\n", condicion ? " OK " : "FALLA", descripcion);
    if (!condicion)
        ++fallos;
}

static bool cerca(double a, double b, double tolerancia)
{
    return qAbs(a - b) <= tolerancia;
}

int main()
{
    const double dt = 1.0 / 60.0;

    // 1. Tiro parabolico: alcance = v0^2 sin(2a) / g
    {
        MovimientoParabolico m(400.0, qDegreesToRadians(45.0), 600.0);
        double alturaMax = 0.0;
        while (!m.aterrizo()) {
            m.paso(dt);
            alturaMax = qMax(alturaMax, m.altura());
        }
        double alcanceTeorico = 400.0 * 400.0 / 600.0;                 // sin(90) = 1
        double alturaTeorica = qPow(400.0 * qSin(M_PI / 4), 2) / (2 * 600.0);
        verificar(cerca(m.distancia(), alcanceTeorico, 1.0), "parabolico: alcance = v0^2 sin(2a)/g");
        verificar(cerca(alturaMax, alturaTeorica, 2.0), "parabolico: altura maxima = (v0 sin a)^2 / 2g");
        verificar(m.altura() == 0.0, "parabolico: termina en el suelo");
    }

    // 2. Pendulo: vuelve al angulo inicial despues de un periodo
    {
        MovimientoPendular p(0.5, 4.0, 9.8);
        double T = p.periodo();
        verificar(cerca(T, 2 * M_PI * qSqrt(4.0 / 9.8), 1e-9), "pendular: T = 2 pi sqrt(L/g)");
        p.paso(T / 2.0);
        verificar(cerca(p.anguloActual(), -0.5, 1e-6), "pendular: a medio periodo esta en -theta0");
        p.paso(T / 2.0);
        verificar(cerca(p.anguloActual(), 0.5, 1e-6), "pendular: tras un periodo vuelve a theta0");
    }

    // 3. Fluido: sin fuerza de Carlos, la velocidad tiende a k vr / (k + b)
    {
        MovimientoFluido f(1.0, 2.5, 0.8, 120.0);
        for (int i = 0; i < 60 * 10; ++i)
            f.paso(dt);
        double vt = 2.5 * 120.0 / (2.5 + 0.8);
        verificar(cerca(f.getVelocidad().x(), vt, 0.5), "fluido: llega a la velocidad terminal");
        verificar(cerca(f.velocidadTerminal().x(), vt, 1e-9), "fluido: formula de velocidad terminal");

        f.reiniciar();
        f.setFuerzaExterna(QPointF(-400.0, 0.0));   // nadando contra la corriente
        for (int i = 0; i < 60 * 10; ++i)
            f.paso(dt);
        verificar(f.getVelocidad().x() < 0.0, "fluido: con fuerza suficiente avanza contra la corriente");
    }

    // 4. Oscilatorio: y = A sin(wt + fase)
    {
        MovimientoOscilatorio o(10.0, 2.0, 0.0);
        double t = M_PI / 4.0;                       // w t = pi/2 -> y = A
        verificar(cerca(o.paso(t).y(), 10.0, 1e-9), "oscilatorio: maximo = A");
        o.setAmplitud(20.0);
        verificar(cerca(o.paso(0).y(), 20.0, 1e-9), "oscilatorio: la amplitud se puede cambiar (creciente)");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS FALLANDO", fallos);
    return fallos == 0 ? 0 : 1;
}
