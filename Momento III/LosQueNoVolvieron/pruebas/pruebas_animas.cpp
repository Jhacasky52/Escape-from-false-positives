// Pruebas automaticas del agente inteligente (las animas), sin graficos.
#include "agente/Razonamiento.h"
#include "agente/Aprendizaje.h"
#include "agente/Anima.h"
#include "agente/AnimaRamon.h"
#include "control/Juego.h"
#include "niveles/NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"

#include <QCoreApplication>
#include <QtMath>
#include <cstdio>

static int fallos = 0;
static const double DT = 1.0 / 60.0;

static void verificar(bool condicion, const char *descripcion)
{
    std::printf("[%s] %s\n", condicion ? " OK " : "FALLA", descripcion);
    if (!condicion)
        ++fallos;
}

// Estimulo de prueba: Carlos en la celda (5,5) rodeado de celdas libres, meta a la derecha
static Estimulo estimuloBase()
{
    Estimulo e;
    e.tamCelda = 64.0;
    e.celdaCarlos = QPoint(5, 5);
    e.posCarlos = QPointF(5.5 * 64, 5.5 * 64);
    e.meta = QRectF(20 * 64, 0, 128, 24 * 64);
    for (int df = -3; df <= 3; ++df)
        for (int dc = -3; dc <= 3; ++dc) {
            CeldaPercibida c;
            c.celda = QPoint(5 + dc, 5 + df);
            c.centro = QPointF((5 + dc + 0.5) * 64, (5 + df + 0.5) * 64);
            c.exposicion = 1.0;
            c.escondite = false;
            e.celdasVecinas.append(c);
        }
    return e;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // ---------------- Razonamiento ----------------
    {
        Razonamiento r;
        Estimulo e = estimuloBase();
        Decision d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::GUIAR, "razonamiento: sin peligro decide GUIAR");
        verificar(d.destino.x() > e.posCarlos.x(), "razonamiento: guia hacia la meta");

        e.panicoCarlos = 80.0;
        d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::CALMAR, "razonamiento: con panico alto decide CALMAR");

        e = estimuloBase();
        e.posSoldados.append(e.posCarlos + QPointF(60, 0));
        e.soldadosPersiguiendo.append(false);
        d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::OCULTARSE, "razonamiento: soldado encima decide OCULTARSE");

        // Una linterna apunta a Carlos desde 200 px a la derecha
        e = estimuloBase();
        e.posSoldados.append(e.posCarlos + QPointF(200, 0));
        e.soldadosPersiguiendo.append(false);
        e.zonasIluminadas.append({e.posCarlos + QPointF(200, 0), M_PI, 230.0, qDegreesToRadians(21.0)});
        d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::ADVERTIR, "razonamiento: Carlos en la luz decide ADVERTIR");
        verificar(r.getUltimoRiesgo().value(d.celdaDestino) < r.getUltimoRiesgo().value(e.celdaCarlos),
                  "razonamiento: el refugio tiene menos riesgo que donde esta Carlos");
    }

    // ---------------- Aprendizaje ----------------
    {
        Aprendizaje mem;
        verificar(mem.peligroAprendido(QPoint(6, 5)) == 0.0, "aprendizaje: sin capturas no hay peligro");
        mem.registrarCaptura(QPoint(6, 5));
        verificar(mem.peligroAprendido(QPoint(6, 5)) > 0.9, "aprendizaje: la zona de la captura queda peligrosa");
        verificar(mem.peligroAprendido(QPoint(7, 5)) > 0.3, "aprendizaje: las vecinas tambien");

        // Con la memoria, el anima ya no guia hacia la zona donde lo atraparon
        Razonamiento r;
        Estimulo e = estimuloBase();
        Decision antes = r.decidir(e, nullptr);
        Aprendizaje mem2;
        mem2.registrarCaptura(antes.celdaDestino);
        Decision despues = r.decidir(e, &mem2);
        verificar(despues.celdaDestino != antes.celdaDestino,
                  "aprendizaje: evita la zona de la captura en el siguiente intento");

        Aprendizaje mem3;
        for (int i = 0; i < 10; ++i)
            mem3.registrarRespuesta(Decision(), false);
        verificar(mem3.insistencia() > 0.7, "aprendizaje: si Carlos las ignora, se vuelven insistentes");
        for (int i = 0; i < 20; ++i)
            mem3.registrarRespuesta(Decision(), true);
        verificar(mem3.insistencia() < 0.1, "aprendizaje: si les hace caso, se calman");
    }

    // ---------------- Integracion con el nivel ----------------
    {
        Juego juego;
        NivelSelva *nivel = new NivelSelva();
        juego.agregarNivel(nivel);
        juego.iniciar();
        verificar(nivel->getAnimas().size() == 4, "nivel: 3 animas + Ramon");

        // Las animas acompanan a Carlos
        Carlos *c = nivel->getCarlos();
        QPointF inicio = nivel->getAnimas().first()->getPosicion();
        for (int i = 0; i < 60 * 3; ++i)
            juego.actualizar(DT);
        QPointF ahora = nivel->getAnimas().first()->getPosicion();
        QPointF d = ahora - c->getPosicion();
        verificar(ahora != inicio && qSqrt(d.x() * d.x() + d.y() * d.y()) < 300.0,
                  "accion: las animas se mueven y se quedan cerca de Carlos");

        // Calmar: con panico alto y las animas cerca, baja mas rapido
        c->aumentarPanico(90.0);
        for (int i = 0; i < 60 * 2; ++i)
            juego.actualizar(DT);
        verificar(c->getPanico() < 75.0, "accion: las animas calman a Carlos");

        // Ramon aparece solo por momentos
        bool vistoVisible = false, vistoOculto = false;
        AnimaRamon *ramon = dynamic_cast<AnimaRamon *>(nivel->getAnimas().last());
        for (int i = 0; i < 60 * 35; ++i) {
            ramon->actualizar(DT, nivel);
            if (ramon->esVisible()) vistoVisible = true; else vistoOculto = true;
        }
        verificar(ramon != nullptr && vistoVisible && vistoOculto, "ramon: aparece y desaparece");

        // Captura: la memoria la guarda y sobrevive al reinicio del nivel
        Soldado *s = nivel->getSoldados().first();
        bool persiguio = false;
        for (int i = 0; i < 60 * 10 && juego.getEstado() == EstadoJuego::JUGANDO; ++i) {
            if (!persiguio) {
                double dir = s->getOrientacion();
                c->setPosicion(s->getPosicion() + QPointF(qCos(dir), qSin(dir)) * 80.0);
            }
            juego.actualizar(DT);
            persiguio = persiguio || s->getEstado() == EstadoSoldado::PERSIGUIENDO;
        }
        verificar(juego.getEstado() == EstadoJuego::CAPTURADO, "juego: Carlos es capturado");
        verificar(juego.getMemoriaAnimas()->numeroCapturas() == 1, "juego: la captura queda en la memoria");
        juego.reiniciarNivel();
        verificar(juego.getMemoriaAnimas()->numeroCapturas() == 1
                  && nivel->getMemoria() == juego.getMemoriaAnimas(),
                  "juego: la memoria sobrevive al reiniciar el nivel");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS FALLANDO", fallos);
    return fallos == 0 ? 0 : 1;
}
