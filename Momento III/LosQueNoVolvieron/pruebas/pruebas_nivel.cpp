// Pruebas automaticas de las reglas del nivel 1 (sin graficos).
#include "niveles/NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include "entidades/Piedra.h"

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

    // ---------------- Fase 3 ----------------

    // 7. Una piedra sigue la parabola, cae y el soldado cercano va a investigar
    {
        nivel.reiniciar();
        Soldado *s = nivel.getSoldados().first();
        Carlos *c = nivel.getCarlos();
        // Carlos 200 px debajo del soldado, mirando hacia el, lanza a media fuerza
        c->setPosicion(s->getPosicion() + QPointF(0, 200));
        c->setDireccion(QPointF(0, -1));
        nivel.actualizar(DT);
        c->setDireccion(QPointF());
        verificar(nivel.lanzarPiedra(0.4), "piedra: Carlos puede lanzar");
        verificar(!nivel.lanzarPiedra(0.4), "piedra: hay que esperar para lanzar otra");
        double alturaMax = 0.0;
        bool investigo = false;
        for (int i = 0; i < 60 * 3; ++i) {
            nivel.actualizar(DT);
            if (!nivel.getPiedras().isEmpty())
                alturaMax = qMax(alturaMax, nivel.getPiedras().last()->getAltura());
            if (s->getEstado() == EstadoSoldado::INVESTIGANDO)
                investigo = true;
        }
        verificar(alturaMax > 20.0, "piedra: sube en parabola antes de caer");
        verificar(nivel.getPiedras().last()->cayo(), "piedra: termina en el suelo");
        verificar(investigo, "ruido: el soldado va a INVESTIGAR donde cayo la piedra");
    }

    // 8. Camino con busqueda en anchura: rodea los arboles
    {
        QPointF a((1 + 0.5) * 64, (12 + 0.5) * 64), b((36 + 0.5) * 64, (12 + 0.5) * 64);
        QVector<QPointF> camino = nivel.buscarCamino(a, b);
        bool libre = !camino.isEmpty();
        for (const QPointF &p : camino)
            if (nivel.celdaEnPunto(p) == Celda::ARBOL)
                libre = false;
        verificar(libre, "camino: cruza el mapa sin pasar por arboles");
    }

    // 9. ConocedorDelMonte: escondido nadie lo ve; en la vegetacion lo ven menos
    {
        nivel.reiniciar();
        Carlos *c = nivel.getCarlos();
        QPointF matorral;
        for (int f = 0; f < nivel.getFilas() && matorral.isNull(); ++f)
            for (int col = 0; col < nivel.getColumnas(); ++col)
                if (nivel.celdaEn(col, f) == Celda::MATORRAL) {
                    matorral = QPointF((col + 0.5) * 64, (f + 0.5) * 64);
                    break;
                }
        c->setPosicion(matorral);
        nivel.actualizar(DT);
        verificar(c->getVisibilidad() < 0.5, "conocedor: en el matorral su visibilidad baja");
        verificar(c->esconderse(), "conocedor: se puede esconder en el matorral");
        nivel.actualizar(DT);
        verificar(c->getVisibilidad() == 0.0, "conocedor: escondido es invisible");
        for (int i = 0; i < 60 * 7; ++i)
            nivel.actualizar(DT);
        verificar(!c->estaEscondido(), "conocedor: tiene que salir del escondite al acabarse el tiempo");

        c->setPosicion(QPointF(1.5 * 64, 12.5 * 64));   // celda C, sobre la trocha
        QPointF antes = c->getPosicion();
        c->setDireccion(QPointF(1, 0));
        nivel.actualizar(DT);
        double recorrido = c->getPosicion().x() - antes.x();
        c->setDireccion(QPointF());
        verificar(recorrido > 140.0 * DT * 1.1, "conocedor: en la trocha camina mas rapido");
    }

    // 10. Miedoso: el panico sube con un soldado cerca y baja al estar quieto lejos
    {
        nivel.reiniciar();
        Carlos *c = nivel.getCarlos();
        Soldado *s = nivel.getSoldados().first();
        for (int i = 0; i < 60 * 2; ++i) {
            c->setPosicion(s->getPosicion() + QPointF(0, -120));
            nivel.actualizar(DT);
        }
        double panicoCerca = c->getPanico();
        verificar(panicoCerca > 10.0, "miedoso: el panico sube con un soldado cerca");
        c->setPosicion(QPointF(1.5 * 64, 12.5 * 64));
        for (int i = 0; i < 60 * 3; ++i)
            nivel.actualizar(DT);
        verificar(c->getPanico() < panicoCerca, "miedoso: lejos y quieto se calma");
        verificar(c->ritmoLatidos() >= 60.0, "miedoso: los latidos dependen del panico");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS FALLANDO", fallos);
    return fallos == 0 ? 0 : 1;
}
