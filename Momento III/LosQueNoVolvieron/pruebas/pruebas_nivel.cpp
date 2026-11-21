// Pruebas automaticas de las reglas del nivel 1 (sin graficos).
#include "niveles/NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"

#include <QCoreApplication>
#include <QtMath>
#include <cstdio>

static int fallos = 0;

static void verificar(bool condicion, const char *descripcion)
{
    std::printf("[%s] %s\n", condicion ? " OK " : "FALLA", descripcion);
    if (!condicion)
        ++fallos;
}

static const double DT = 1.0 / 60.0;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 1. El mapa se carga desde los recursos
    NivelSelva nivel;
    nivel.cargar();
    verificar(nivel.getColumnas() == 40 && nivel.getFilas() == 24, "mapa: 40 x 24 celdas");
    verificar(nivel.getCarlos() != nullptr, "mapa: Carlos aparece en la celda C");
    verificar(nivel.getSoldados().size() == 4, "mapa: 4 soldados, uno por RUTA");

    // 2. Los arboles bloquean el paso: el borde izquierdo es una fila de arboles
    {
        Carlos *c = nivel.getCarlos();
        c->setPosicion(QPointF(1.5 * 64, 12.5 * 64));
        c->setDireccion(QPointF(-1, 0));
        for (int i = 0; i < 120; ++i)
            nivel.actualizar(DT);
        verificar(c->getPosicion().x() > 64.0, "colision: Carlos no atraviesa los arboles");
        c->setDireccion(QPointF());
    }

    // 3. Linterna: ilumina lo que esta dentro del cono y no lo que esta detras
    {
        Linterna l(230.0, qDegreesToRadians(21.0), 0.0, 4.0);   // sin barrido
        l.setOrigen(QPointF(0, 0));
        l.setDireccionBase(0.0);                                 // mira a la derecha
        verificar(l.ilumina(QPointF(150, 10)), "linterna: ilumina un punto delante");
        verificar(!l.ilumina(QPointF(-150, 0)), "linterna: no ilumina detras");
        verificar(!l.ilumina(QPointF(300, 0)), "linterna: no ilumina mas alla del alcance");
        auto pared = [](const QPointF &p) { return p.x() > 50 && p.x() < 70; };
        verificar(!l.ilumina(QPointF(150, 0), pared), "linterna: un arbol tapa la luz");
    }

    // 4. Si Carlos se queda en la luz lo descubren, lo persiguen y lo atrapan
    {
        nivel.reiniciar();
        Soldado *s = nivel.getSoldados().first();
        Carlos *c = nivel.getCarlos();
        bool persiguio = false;
        for (int i = 0; i < 60 * 8 && !nivel.haPerdido(); ++i) {
            // Carlos se queda delante de la linterna hasta que lo descubren;
            // despues se queda quieto y el soldado va por el
            if (!persiguio) {
                const Linterna *l = s->getLinterna();
                double d = l->direccionActual();
                c->setPosicion(s->getPosicion() + QPointF(qCos(d), qSin(d)) * 90.0);
            }
            nivel.actualizar(DT);
            if (s->getEstado() == EstadoSoldado::PERSIGUIENDO)
                persiguio = true;
        }
        verificar(persiguio, "deteccion: el soldado pasa a PERSIGUIENDO");
        verificar(nivel.haPerdido(), "captura: el soldado alcanza a Carlos y el nivel se pierde");
    }

    // 5. Reiniciar limpia la captura
    nivel.reiniciar();
    verificar(!nivel.haPerdido(), "reinicio: el nivel vuelve a empezar");

    // 6. Llegar al rio gana el nivel
    {
        Carlos *c = nivel.getCarlos();
        c->setPosicion(QPointF(nivel.getMeta().left() + 20, 12.5 * 64));
        verificar(nivel.haGanado(), "meta: llegar al rio gana el nivel");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS FALLANDO", fallos);
    return fallos == 0 ? 0 : 1;
}
