// Pruebas automaticas de la dificultad (nivel 1, la selva), sin graficos.
// Cada prueba muestra que un eje de la dificultad cambia un COMPORTAMIENTO.
#include "control/Dificultad.h"
#include "control/Juego.h"
#include "control/Partida.h"
#include "niveles/NivelSelva.h"
#include "niveles/NivelRio.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include "agente/Anima.h"
#include "agente/AnimaRamon.h"
#include "agente/Razonamiento.h"
#include "agente/Aprendizaje.h"
#include "rasgos/Miedoso.h"
#include "rasgos/ConocedorDelMonte.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
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

static double distancia(const QPointF &a, const QPointF &b)
{
    QPointF d = a - b;
    return qSqrt(d.x() * d.x() + d.y() * d.y());
}

// Segundos que tarda un soldado quieto en descubrir a Carlos parado frente a su linterna
static double tiempoEnDescubrir(NivelDificultad n)
{
    Soldado s({QPointF(0, 0)});
    s.configurar(Dificultad::crear(n));
    Carlos c(QPointF(0, 0));
    for (int i = 1; i < 60 * 10; ++i) {
        s.actualizar(DT);
        double d = s.getLinterna()->direccionActual();
        c.setPosicion(QPointF(qCos(d), qSin(d)) * 100.0);
        s.detecta(&c, DT, nullptr);
        if (s.getEstado() == EstadoSoldado::PERSIGUIENDO)
            return i * DT;
    }
    return 99.0;
}

// Selva con la dificultad dada; el soldado 0 descubre a Carlos.
// Devuelve cuantos otros soldados, dentro del radio de aviso, salieron a investigar.
static int soldadosQueAcuden(NivelDificultad n, int *cercanos)
{
    NivelSelva nivel;
    nivel.setDificultad(Dificultad::crear(n));
    nivel.cargar();
    Soldado *s = nivel.getSoldados().first();
    Carlos *c = nivel.getCarlos();
    for (int i = 0; i < 60 * 6 && s->getEstado() != EstadoSoldado::PERSIGUIENDO; ++i) {
        double d = s->getLinterna()->direccionActual();
        c->setPosicion(s->getPosicion() + QPointF(qCos(d), qSin(d)) * 90.0);
        nivel.actualizar(DT);
    }
    int acuden = 0;
    *cercanos = 0;
    for (Soldado *otro : nivel.getSoldados()) {
        if (otro == s || distancia(otro->getPosicion(), s->getPosicion()) > Dificultad::crear(NivelDificultad::DIFICIL).radioAviso)
            continue;
        ++*cercanos;
        if (otro->getEstado() == EstadoSoldado::INVESTIGANDO)
            ++acuden;
    }
    return acuden;
}

// Carlos escondido en un matorral y un soldado revisando justo ahi. ¿Lo atrapan en 3 s?
static bool loEncuentranEscondido(NivelDificultad n)
{
    NivelSelva nivel;
    nivel.setDificultad(Dificultad::crear(n));
    nivel.cargar();
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
    c->esconderse();
    Soldado *s = nivel.getSoldados().first();
    s->setPosicion(matorral + QPointF(30, 0));
    s->escucharRuido(matorral + QPointF(30, 0), 1.0);
    for (int i = 0; i < 60 * 3 && !nivel.haPerdido(); ++i)
        nivel.actualizar(DT);
    return nivel.haPerdido();
}

// Punto que revisa el soldado cuando pierde a Carlos que corria hacia la derecha
static QPointF puntoAlPerderlo(NivelDificultad n, QPointF *ultimaVez)
{
    Soldado s({QPointF(0, 0)});
    s.configurar(Dificultad::crear(n));
    Carlos c(QPointF(0, 0));
    c.setDireccion(QPointF(1, 0));
    for (int i = 0; i < 60 * 5 && s.getEstado() != EstadoSoldado::PERSIGUIENDO; ++i) {
        s.actualizar(DT);
        c.actualizar(DT);   // se mueve: queda con velocidad hacia la derecha
        double d = s.getLinterna()->direccionActual();
        c.setPosicion(s.getPosicion() + QPointF(qCos(d), qSin(d)) * 100.0);
        s.detecta(&c, DT, nullptr);
    }
    *ultimaVez = s.getPuntoSospechoso();
    c.setPosicion(QPointF(5000, 5000));   // se escapa: ya no esta en la luz
    for (int i = 0; i < 60 * 8 && s.getEstado() == EstadoSoldado::PERSIGUIENDO; ++i) {
        s.actualizar(DT);
        s.detecta(&c, DT, nullptr);
    }
    return s.getPuntoSospechoso();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Dificultad f = Dificultad::crear(NivelDificultad::FACIL);
    Dificultad n = Dificultad::crear(NivelDificultad::NORMAL);
    Dificultad d = Dificultad::crear(NivelDificultad::DIFICIL);

    // ---------------- Los ejes estan ordenados ----------------
    verificar(f.tiempoParaDescubrir > n.tiempoParaDescubrir && n.tiempoParaDescubrir > d.tiempoParaDescubrir,
              "ejes: en dificil descubren mas rapido");
    verificar(f.radioPercepcionAnimas > n.radioPercepcionAnimas && n.radioPercepcionAnimas > d.radioPercepcionAnimas,
              "ejes: en facil las animas ven mas lejos");
    verificar(f.umbralPanico > n.umbralPanico && n.umbralPanico > d.umbralPanico,
              "ejes: en dificil el panico afecta antes");
    verificar(!f.busquedaPredictiva && !n.busquedaPredictiva && d.busquedaPredictiva && d.revisaEscondites
              && f.radioAviso == 0.0 && d.radioAviso > 0.0,
              "ejes: los comportamientos extra solo estan en dificil");
    Dificultad porDefecto;
    verificar(porDefecto.tiempoParaDescubrir == n.tiempoParaDescubrir && porDefecto.radioAviso == n.radioAviso,
              "ejes: Normal es el juego tal como era antes");

    // ---------------- Eje 1: percepcion ----------------
    double tf = tiempoEnDescubrir(NivelDificultad::FACIL);
    double tn = tiempoEnDescubrir(NivelDificultad::NORMAL);
    double td = tiempoEnDescubrir(NivelDificultad::DIFICIL);
    std::printf("       descubren a Carlos en: facil %.2f s, normal %.2f s, dificil %.2f s\n", tf, tn, td);
    verificar(tf > tn && tn > td, "percepcion: el mismo soldado tarda menos en descubrirlo en dificil");
    {
        Soldado sf({QPointF(0, 0)}), sn({QPointF(0, 0)}), sd({QPointF(0, 0)});
        sf.configurar(f);
        sn.configurar(n);
        sd.configurar(d);
        sf.escucharRuido(QPointF(340, 0), 1.0);
        sn.escucharRuido(QPointF(340, 0), 1.0);
        sd.escucharRuido(QPointF(440, 0), 1.0);
        verificar(sf.getEstado() == EstadoSoldado::PATRULLANDO && sn.getEstado() == EstadoSoldado::INVESTIGANDO,
                  "percepcion: una piedra a 340 px la oye en normal pero no en facil");
        verificar(sd.getEstado() == EstadoSoldado::INVESTIGANDO, "percepcion: en dificil oye hasta 475 px");
    }

    // ---------------- Eje 2: inteligencia (comportamientos) ----------------
    {
        int cercanosF = 0, cercanosD = 0;
        int acudenF = soldadosQueAcuden(NivelDificultad::FACIL, &cercanosF);
        int acudenD = soldadosQueAcuden(NivelDificultad::DIFICIL, &cercanosD);
        std::printf("       soldados cerca del que lo vio: %d; acuden en facil %d, en dificil %d\n",
                    cercanosD, acudenF, acudenD);
        verificar(cercanosD > 0 && acudenD == cercanosD, "inteligencia: en dificil los soldados cercanos acuden al aviso");
        verificar(acudenF == 0, "inteligencia: en facil cada soldado actua solo");

        verificar(loEncuentranEscondido(NivelDificultad::DIFICIL),
                  "inteligencia: en dificil revisan el escondite y lo encuentran");
        verificar(!loEncuentranEscondido(NivelDificultad::NORMAL),
                  "inteligencia: en normal el escondite lo protege");

        QPointF vistoN, vistoD;
        QPointF buscaN = puntoAlPerderlo(NivelDificultad::NORMAL, &vistoN);
        QPointF buscaD = puntoAlPerderlo(NivelDificultad::DIFICIL, &vistoD);
        verificar(distancia(buscaN, vistoN) < 1.0, "inteligencia: en normal busca donde lo vio por ultima vez");
        verificar(buscaD.x() > vistoD.x() + 100.0, "inteligencia: en dificil busca hacia donde iba (prediccion)");
    }

    // ---------------- Eje 3: ayuda de las animas ----------------
    {
        NivelSelva facil, dificil;
        facil.setDificultad(f);
        dificil.setDificultad(d);
        facil.cargar();
        dificil.cargar();
        for (int i = 0; i < 60 * 2; ++i) {
            facil.actualizar(DT);
            dificil.actualizar(DT);
        }
        bool tenues = true, umbrales = true;
        for (Anima *a : dificil.getAnimas())
            tenues = tenues && a->getBrillo() <= d.brilloMaximoAnimas + 1e-9;
        for (Anima *a : facil.getAnimas())
            umbrales = umbrales && qAbs(a->getRazonamiento()->getUmbralRiesgo() - f.umbralAdvertencia) < 1e-9;
        verificar(tenues, "animas: en dificil nunca brillan del todo");
        verificar(umbrales, "animas: en facil advierten con menos riesgo");
        double periodoF = 0, periodoD = 0;
        for (Anima *a : facil.getAnimas())
            if (auto *r = dynamic_cast<AnimaRamon *>(a)) periodoF = r->getPeriodo();
        for (Anima *a : dificil.getAnimas())
            if (auto *r = dynamic_cast<AnimaRamon *>(a)) periodoD = r->getPeriodo();
        verificar(periodoF < periodoD, "animas: Ramon aparece mas seguido en facil");
    }

    // ---------------- Eje 4: margen de error ----------------
    {
        NivelSelva nivel;
        nivel.setDificultad(d);
        nivel.cargar();
        double umbral = 0, escondite = 0;
        for (Rasgo *r : nivel.getCarlos()->getRasgos()) {
            if (auto *m = dynamic_cast<Miedoso *>(r)) umbral = m->getUmbralPanico();
            if (auto *c = dynamic_cast<ConocedorDelMonte *>(r)) escondite = c->getTiempoMaxEscondido();
        }
        verificar(umbral == d.umbralPanico && escondite == d.tiempoMaxEscondido,
                  "margen: en dificil tiembla antes y aguanta menos escondido");
    }

    // ---------------- Juego y partida ----------------
    {
        QTemporaryDir carpeta;
        QString ruta = carpeta.path() + "/partida.json";
        Juego j;
        NivelSelva *selva = new NivelSelva();
        j.agregarNivel(selva);
        j.agregarNivel(new NivelRio());
        j.setDificultad(NivelDificultad::DIFICIL);
        j.iniciar();
        verificar(selva->getDificultad().nivel == NivelDificultad::DIFICIL, "juego: la dificultad llega al nivel");
        verificar(Partida::guardar(j, ruta), "partida: se guarda con la dificultad");
        Juego j2;
        j2.agregarNivel(new NivelSelva());
        j2.agregarNivel(new NivelRio());
        j2.iniciar();
        verificar(Partida::cargar(j2, ruta) && j2.getDificultad().nivel == NivelDificultad::DIFICIL,
                  "partida: al cargar vuelve la misma dificultad");

        QString vieja = carpeta.path() + "/vieja.json";
        QFile v(vieja);
        v.open(QIODevice::WriteOnly);
        v.write("{\"version\": 1, \"nivel\": 0, \"animas\": {}}");
        v.close();
        verificar(Partida::cargar(j2, vieja) && j2.getDificultad().nivel == NivelDificultad::NORMAL,
                  "partida: una partida sin dificultad (de antes) carga en Normal");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS QUE FALLAN", fallos);
    return fallos == 0 ? 0 : 1;
}
