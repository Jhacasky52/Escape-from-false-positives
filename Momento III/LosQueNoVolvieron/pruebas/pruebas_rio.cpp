// Pruebas automaticas del nivel 2 (el rio), sin graficos.
#include "niveles/NivelRio.h"
#include "niveles/NivelSelva.h"
#include "control/Juego.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Tronco.h"
#include "agente/Razonamiento.h"
#include "agente/Anima.h"
#include "agente/Aprendizaje.h"

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

static void avanzar(NivelRio &n, double segundos)
{
    for (int i = 0; i < qRound(segundos / DT) && !n.haPerdido(); ++i)
        n.actualizar(DT);
}

// Carlos en el agua en la posicion x (en la superficie), sin soldados
static void meterAlAgua(NivelRio &n, double x)
{
    n.retirarSoldados();
    n.setGenerarTroncos(false);
    Carlos *c = n.getCarlos();
    c->entrarAlAgua();
    c->setPosicion(QPointF(x, n.getSuperficie()));
    n.actualizar(DT);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Aprendizaje memoria;

    // ---------------- Carga ----------------
    {
        NivelRio n;
        n.setMemoria(&memoria);
        n.cargar();
        Carlos *c = n.getCarlos();
        verificar(c != nullptr && !c->estaEnAgua(), "carga: Carlos empieza en tierra");
        verificar(n.getApoyoActual() == 0, "carga: parado en la orilla de salida");
        verificar(n.getSoldados().size() == 3, "carga: tres soldados en la orilla");
        verificar(n.getAnimas().size() == 4, "carga: tres animas sobre las rocas altas y Ramon");
        verificar(n.getEtapa() == EtapaRio::NORMAL, "carga: el rio empieza normal");
        verificar(!n.haGanado() && !n.haPerdido(), "carga: ni gana ni pierde al empezar");
    }

    // ---------------- Caminar fuera de la orilla ----------------
    {
        NivelRio n;
        n.cargar();
        n.retirarSoldados();
        Carlos *c = n.getCarlos();
        c->setDireccion(QPointF(1, 0));
        avanzar(n, 1.5);   // 140 px/s: sale de la orilla (x = 260)
        verificar(c->estaEnAgua(), "tierra: al pasar el borde de la orilla cae al agua");
        verificar(n.getApoyoActual() == -1, "tierra: ya no pisa ningun apoyo");
    }

    // ---------------- Fisica del agua ----------------
    {
        NivelRio n;
        n.cargar();
        meterAlAgua(n, 2000);
        Carlos *c = n.getCarlos();
        double x0 = c->getPosicion().x();
        avanzar(n, 3.0);
        verificar(c->getPosicion().x() < x0 - 100, "agua: sin nadar la corriente lo arrastra a la izquierda");
        verificar(qAbs(c->profundidad()) < 10.0, "agua: flota en la superficie");

        // Nadando contra la corriente: v terminal = (F + k vr) / (k + b) = (480 - 3*70) / 4
        c->setDireccion(QPointF(1, 0));
        avanzar(n, 4.0);
        double esperada = (480.0 - 3.0 * 70.0) / 4.0;
        double vx = c->getVelocidad().x();
        std::printf("       vx = %.1f px/s (esperada %.1f)\n", vx, esperada);
        verificar(qAbs(vx - esperada) < 5.0, "agua: nadando llega a la velocidad terminal del modelo");
    }

    // ---------------- Bucear y aire ----------------
    {
        NivelRio n;
        n.cargar();
        meterAlAgua(n, 2000);
        Carlos *c = n.getCarlos();
        c->setDireccion(QPointF(0, 1));
        avanzar(n, 1.5);
        verificar(c->estaSumergido(), "bucear: con abajo se sumerge");
        verificar(c->getVisibilidad() == 0.0, "bucear: sumergido no se ve");
        verificar(c->getAire() < 100.0, "bucear: gasta aire");
        verificar(n.esEscondite(c->getPosicion()) || c->profundidad() > Carlos::PROF_OCULTO,
                  "bucear: el fondo cuenta como escondite para las animas");
        double aire = c->getAire();
        c->setDireccion(QPointF());
        avanzar(n, 2.5);
        verificar(!c->estaSumergido() && c->getAire() > aire, "bucear: al salir a respirar recupera aire");

        // Buceando en el mismo lugar (se le devuelve la x para que no lo arrastre la corriente)
        c->setDireccion(QPointF(0, 1));
        for (int i = 0; i < 12 * 60 && !n.haPerdido(); ++i) {
            c->setPosicion(QPointF(2000, c->getPosicion().y()));
            n.actualizar(DT);
        }
        verificar(n.haPerdido() && n.getMotivo() == MotivoDerrota::AHOGADO,
                  "bucear: si se queda sin aire se ahoga");
    }

    // ---------------- Creciente ----------------
    {
        NivelRio n;
        n.cargar();
        n.setTiempo(10.0);
        verificar(qAbs(n.tiempoParaCreciente() - 10.0) < 0.01, "creciente: faltan 10 s a los 10 s del ciclo");
        n.setTiempo(30.0);
        verificar(n.getEtapa() == EtapaRio::CRECIENTE, "creciente: a los 30 s el rio esta crecido");
        verificar(qAbs(n.getSuperficie() - NivelRio::SUPERFICIE_CRECIENTE) < 0.01, "creciente: el agua sube");
        verificar(qAbs(n.getVelocidadRio() + 150.0) < 0.01, "creciente: la corriente pasa de -70 a -150 px/s");
        n.setTiempo(24.0);
        verificar(n.getEtapa() == EtapaRio::MEDIA, "creciente: a mitad de la subida esta en etapa media");

        bool bajaTapada = true, altaSeca = true;
        n.setTiempo(30.0);
        for (const Apoyo &a : n.getApoyos()) {
            if (a.tipo == TipoApoyo::ROCA_BAJA || a.tipo == TipoApoyo::BANCO_ARENA)
                bajaTapada = bajaTapada && !n.apoyoSeco(a);
            if (a.tipo == TipoApoyo::ROCA_ALTA || a.tipo == TipoApoyo::ORILLA)
                altaSeca = altaSeca && n.apoyoSeco(a);
        }
        verificar(bajaTapada, "creciente: tapa las rocas bajas y los bancos de arena");
        verificar(altaSeca, "creciente: las rocas altas y las orillas siguen secas");
    }

    // ---------------- Subirse a una roca y que la tape la creciente ----------------
    {
        NivelRio n;
        n.cargar();
        // Roca baja en x 620..730 (arriba en y = 272)
        meterAlAgua(n, 700);
        Carlos *c = n.getCarlos();
        c->setDireccion(QPointF(0, -1));
        avanzar(n, 0.2);
        verificar(!c->estaEnAgua() && n.getApoyoActual() == 1, "roca: con arriba se sube a la roca baja");
        c->setDireccion(QPointF());
        double x = c->getPosicion().x();
        avanzar(n, 2.0);
        verificar(qAbs(c->getPosicion().x() - x) < 0.5, "roca: sobre la roca la corriente no lo arrastra");
        n.setTiempo(26.0);   // el agua ya paso por encima de la roca baja
        avanzar(n, 0.1);
        verificar(c->estaEnAgua(), "roca: cuando la creciente la tapa, Carlos cae al agua");
    }

    // ---------------- Arrastrado fuera de la pantalla ----------------
    {
        NivelRio n;
        n.cargar();
        meterAlAgua(n, 2200);
        avanzar(n, 20.0);
        verificar(n.haPerdido() && n.getMotivo() == MotivoDerrota::ARRASTRADO,
                  "arrastre: si se deja llevar 560 px pierde");
    }

    // ---------------- Troncos ----------------
    {
        NivelRio n;
        n.cargar();
        meterAlAgua(n, 2000);
        Carlos *c = n.getCarlos();
        n.agregarTronco(new Tronco(c->getPosicion() + QPointF(30, 0), TipoTronco::TRONCO, 0.0));
        double panico = c->getPanico();
        n.actualizar(DT);
        verificar(n.getGolpes() == 1, "tronco: golpea a Carlos en la superficie");
        verificar(c->getVelocidad().x() < -100.0, "tronco: lo empuja rio abajo");
        verificar(c->getPanico() > panico + 20.0, "tronco: el golpe le da panico");

        NivelRio n2;
        n2.cargar();
        meterAlAgua(n2, 2000);
        Carlos *c2 = n2.getCarlos();
        c2->setDireccion(QPointF(0, 1));
        avanzar(n2, 1.5);
        n2.agregarTronco(new Tronco(c2->getPosicion() + QPointF(30, 0), TipoTronco::TRONCO, 0.0));
        n2.actualizar(DT);
        verificar(n2.getGolpes() == 0, "tronco: buceando pasa por debajo sin golpe");

        Tronco t(QPointF(500, 300), TipoTronco::ESCOMBROS, 0.0);
        t.setAgua(300, -70);
        double minY = 1e9, maxY = -1e9;
        for (int i = 0; i < 240; ++i) {
            t.actualizar(DT);
            minY = qMin(minY, t.getPosicion().y());
            maxY = qMax(maxY, t.getPosicion().y());
        }
        verificar(maxY - minY > 10.0 && maxY - minY <= 18.01, "tronco: sube y baja con el oleaje (oscilatorio)");
        verificar(t.getPosicion().x() < 500 - 70 * 1.3 * 4 + 1, "tronco: baja con la corriente");
    }

    // ---------------- Soldados de la orilla ----------------
    {
        NivelRio n;
        n.cargar();
        Carlos *c = n.getCarlos();
        Soldado *s = n.getSoldados().first();
        c->entrarAlAgua();
        // Lo dejamos siempre debajo del soldado, en la superficie
        bool visto = false;
        for (int i = 0; i < 6 * 60 && !visto; ++i) {
            c->setPosicion(QPointF(s->getPosicion().x(), n.getSuperficie()));
            n.actualizar(DT);
            visto = n.haPerdido();
        }
        verificar(visto && n.getMotivo() == MotivoDerrota::VISTO, "soldado: si la linterna lo ve en la superficie, lo atrapan");

        NivelRio n2;
        n2.cargar();
        Carlos *c2 = n2.getCarlos();
        Soldado *s2 = n2.getSoldados().first();
        c2->entrarAlAgua();
        c2->setDireccion(QPointF(0, 1));
        for (int i = 0; i < 6 * 60 && !n2.haPerdido(); ++i) {
            c2->setPosicion(QPointF(s2->getPosicion().x(), c2->getPosicion().y()));
            n2.actualizar(DT);
        }
        verificar(!n2.haPerdido() || n2.getMotivo() != MotivoDerrota::VISTO,
                  "soldado: buceando debajo de la luz no lo ven");
    }

    // ---------------- Animas en el rio ----------------
    {
        Razonamiento r;
        Estimulo e;
        e.tamCelda = 64.0;
        e.posCarlos = QPointF(1500, 300);
        e.celdaCarlos = QPoint(23, 4);
        e.meta = QRectF(4600, 0, 200, 600);
        CeldaPercibida cp;
        cp.celda = QPoint(24, 4);
        cp.centro = QPointF(24.5 * 64, 4.5 * 64);
        cp.exposicion = 1.0;
        cp.escondite = false;
        e.celdasVecinas.append(cp);
        e.corriente = -70.0;
        e.tiempoCreciente = 3.0;
        e.hayRefugio = true;
        e.refugio = QPointF(1035, 206);
        Decision d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::ADVERTIR && d.destino == e.refugio,
                  "animas: si viene la creciente avisan y senalan la roca alta");
        e.tiempoCreciente = 15.0;
        d = r.decidir(e, nullptr);
        verificar(d.tipo == TipoDecision::GUIAR, "animas: sin creciente cerca siguen guiando");

        NivelRio n;
        n.cargar();
        QPointF ref;
        verificar(n.refugioCercano(QPointF(1200, 300), ref) && qAbs(ref.x() - 1060) < 31,
                  "animas: el refugio mas cercano a x=1200 es la roca alta de x=980..1090");
    }

    // ---------------- Juego: de la selva al rio ----------------
    {
        Juego juego;
        NivelSelva *selva = new NivelSelva();
        NivelRio *rio = new NivelRio();
        juego.agregarNivel(selva);
        juego.agregarNivel(rio);
        juego.iniciar();
        selva->getCarlos()->setPosicion(selva->getMeta().center());
        juego.actualizar(DT);
        verificar(juego.getEstado() == EstadoJuego::NIVEL_SUPERADO, "juego: al salir de la selva el nivel queda superado");
        juego.pasarAlSiguienteNivel();
        verificar(juego.nivelActual() == rio && juego.getEstado() == EstadoJuego::JUGANDO,
                  "juego: pasa al rio");
        verificar(rio->getCarlos() != nullptr && rio->getMemoria() == juego.getMemoriaAnimas(),
                  "juego: el rio comparte la memoria de las animas");
        rio->getCarlos()->setPosicion(QPointF(4700, 236));
        juego.actualizar(DT);
        verificar(juego.getEstado() == EstadoJuego::FINAL, "juego: al llegar a la otra orilla termina el juego");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS QUE FALLAN", fallos);
    return fallos == 0 ? 0 : 1;
}
