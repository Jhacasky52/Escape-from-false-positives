// Pruebas automaticas de guardar y cargar la partida (JSON), sin graficos.
#include "control/Juego.h"
#include "control/Partida.h"
#include "niveles/NivelSelva.h"
#include "niveles/NivelRio.h"
#include "agente/Aprendizaje.h"
#include "entidades/Carlos.h"

#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QtMath>
#include <cstdio>

static int fallos = 0;

static void verificar(bool condicion, const char *descripcion)
{
    std::printf("[%s] %s\n", condicion ? " OK " : "FALLA", descripcion);
    if (!condicion)
        ++fallos;
}

static void armar(Juego &j)
{
    j.agregarNivel(new NivelSelva());
    j.agregarNivel(new NivelRio());
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTemporaryDir carpeta;
    const QString ruta = carpeta.path() + "/partidas/partida.json";

    // ---------------- Guardar ----------------
    {
        Juego j;
        armar(j);
        j.iniciar();
        Aprendizaje *m = j.getMemoriaAnimas();
        m->registrarCaptura(QPoint(12, 7));
        m->registrarAvistamiento(QPoint(3, 4), 1.0);
        m->setTasaObediencia(0.4);
        j.getMemoriaAnimas();
        // Llega al rio
        j.nivelActual()->getCarlos()->setPosicion(j.nivelActual()->getMeta().center());
        j.actualizar(1.0 / 60.0);
        j.pasarAlSiguienteNivel();

        verificar(!Partida::existe(ruta), "guardar: al comienzo no hay partida");
        verificar(Partida::guardar(j, ruta), "guardar: crea el archivo (y la carpeta)");
        verificar(Partida::existe(ruta), "guardar: el archivo existe");

        // ---------------- Cargar en un juego nuevo ----------------
        Juego j2;
        armar(j2);
        j2.iniciar();
        verificar(Partida::cargar(j2, ruta), "cargar: lee el archivo");
        verificar(j2.getIndiceNivel() == 1, "cargar: vuelve al nivel del rio");
        verificar(j2.getEstado() == EstadoJuego::JUGANDO && j2.nivelActual()->getCarlos() != nullptr,
                  "cargar: el nivel queda listo para jugar");
        Aprendizaje *m2 = j2.getMemoriaAnimas();
        verificar(m2->getMapaPeligro() == m->getMapaPeligro(), "cargar: recupera el mapa de peligro de las animas");
        verificar(m2->numeroCapturas() == 1 && m2->getCapturas().first() == QPoint(12, 7),
                  "cargar: recupera donde lo atraparon");
        verificar(qAbs(m2->getTasaObediencia() - 0.4) < 1e-9, "cargar: recupera la obediencia (insistencia)");
        verificar(qAbs(m2->peligroAprendido(QPoint(12, 7)) - m->peligroAprendido(QPoint(12, 7))) < 1e-9,
                  "cargar: las animas evitan las mismas zonas que antes");
    }

    // ---------------- Archivos malos ----------------
    {
        Juego j;
        armar(j);
        j.iniciar();
        verificar(!Partida::cargar(j, carpeta.path() + "/no_existe.json"), "cargar: sin archivo devuelve false");

        QString danado = carpeta.path() + "/danado.json";
        QFile f(danado);
        f.open(QIODevice::WriteOnly);
        f.write("{ esto no es json");
        f.close();
        verificar(!Partida::cargar(j, danado), "cargar: un archivo danado devuelve false");

        QString nivelMalo = carpeta.path() + "/nivel_malo.json";
        QFile g(nivelMalo);
        g.open(QIODevice::WriteOnly);
        g.write("{\"version\": 1, \"nivel\": 7, \"animas\": {}}");
        g.close();
        verificar(!Partida::cargar(j, nivelMalo), "cargar: un nivel que no existe devuelve false");
        verificar(j.getIndiceNivel() == 0, "cargar: si falla no cambia el juego");
    }

    // ---------------- Nueva partida y borrar ----------------
    {
        Juego j;
        armar(j);
        j.iniciarEn(1);
        verificar(j.getIndiceNivel() == 1, "juego: iniciarEn(1) empieza en el rio");
        j.getMemoriaAnimas()->registrarCaptura(QPoint(1, 1));
        j.nuevaPartida();
        verificar(j.getIndiceNivel() == 0 && j.getMemoriaAnimas()->numeroCapturas() == 0,
                  "juego: nueva partida empieza en la selva y sin memoria");
        verificar(Partida::borrar(ruta) && !Partida::existe(ruta), "borrar: elimina la partida guardada");
        verificar(Partida::borrar(ruta), "borrar: si no existe no es error");
    }

    std::printf("\n%s (%d fallos)\n", fallos == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS QUE FALLAN", fallos);
    return fallos == 0 ? 0 : 1;
}
