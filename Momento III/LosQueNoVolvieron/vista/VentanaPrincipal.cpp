#include "VentanaPrincipal.h"
#include "EscenaMenu.h"
#include "EscenaNivelSelva.h"
#include "EscenaNivelRio.h"
#include "EscenaFinal.h"
#include "EscenaPruebaFisica.h"
#include "Sonidos.h"
#include "control/Juego.h"
#include "control/Partida.h"
#include "niveles/NivelSelva.h"
#include "niveles/NivelRio.h"

#include <QApplication>
#include <QGraphicsView>
#include <QShortcut>

VentanaPrincipal::VentanaPrincipal(QWidget *parent)
    : QMainWindow(parent), juego(new Juego()), sonidos(new Sonidos(this)),
      rutaPartida(Partida::rutaPorDefecto()), escenaActual(nullptr)
{
    setWindowTitle("Los que no volvieron");

    // Logica: el juego con sus dos niveles (Juego es el dueno)
    NivelSelva *selva = new NivelSelva();
    NivelRio *rio = new NivelRio();
    juego->agregarNivel(selva);
    juego->agregarNivel(rio);
    juego->iniciar();

    // Graficos: menu, una escena por nivel, el final y la prueba de fisicas (F9)
    escenaMenu = new EscenaMenu(this);
    escenaNivel1 = new EscenaNivelSelva(juego, selva, this);
    escenaNivel2 = new EscenaNivelRio(juego, rio, this);
    escenaFinal = new EscenaFinal(this);
    escenaPrueba = new EscenaPruebaFisica(this);
    escenaPrueba->setSceneRect(0, 0, ANCHO, ALTO);

    for (EscenaJuego *e : {static_cast<EscenaJuego *>(escenaMenu), static_cast<EscenaJuego *>(escenaNivel1),
                           static_cast<EscenaJuego *>(escenaNivel2), static_cast<EscenaJuego *>(escenaFinal)}) {
        e->setSonidos(sonidos);
        connect(e, &EscenaJuego::siguienteNivel, this, &VentanaPrincipal::siguienteNivel);
        connect(e, &EscenaJuego::guardarPartida, this, &VentanaPrincipal::guardarPartida);
        connect(e, &EscenaJuego::volverAlMenu, this, &VentanaPrincipal::irAlMenu);
    }
    connect(escenaMenu, &EscenaMenu::nuevaPartida, this, &VentanaPrincipal::nuevaPartida);
    connect(escenaMenu, &EscenaMenu::continuarPartida, this, &VentanaPrincipal::continuarPartida);
    connect(escenaMenu, &EscenaMenu::salir, this, &QWidget::close);

    vista = new QGraphicsView(this);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, false);   // pixel art nitido
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    vista->setFixedSize(ANCHO + 2, ALTO + 2);
    // Sin fondo en la vista: cada escena pinta el suyo (el rio dibuja el cielo con paralaje)
    vista->setBackgroundBrush(Qt::NoBrush);
    setCentralWidget(vista);
    adjustSize();
    setFixedSize(size());

    QShortcut *f9 = new QShortcut(QKeySequence(Qt::Key_F9), this);
    connect(f9, &QShortcut::activated, this, &VentanaPrincipal::alternarPruebaFisica);

    irAlMenu();
}

VentanaPrincipal::~VentanaPrincipal()
{
    // Las escenas se detienen antes de borrar la logica que leen
    for (EscenaJuego *e : {static_cast<EscenaJuego *>(escenaMenu), static_cast<EscenaJuego *>(escenaNivel1),
                           static_cast<EscenaJuego *>(escenaNivel2), static_cast<EscenaJuego *>(escenaFinal),
                           escenaPrueba}) {
        e->detenerBucle();
        delete e;
    }
    delete juego;
}

void VentanaPrincipal::mostrarEscena(EscenaJuego *escena)
{
    if (escenaActual != nullptr)
        escenaActual->detenerBucle();
    escenaActual = escena;
    vista->setScene(escena);
    vista->setFocus();
    escena->iniciarBucle();

    // Sonido de fondo de cada escena
    if (escena == escenaNivel1)
        sonidos->ambiente("ambiente_selva", 0.5);
    else if (escena == escenaNivel2 || escena == escenaMenu)
        sonidos->ambiente("ambiente_rio", escena == escenaMenu ? 0.3 : 0.5);
    else
        sonidos->detenerAmbiente();
}

EscenaJuego *VentanaPrincipal::escenaDelNivel() const
{
    if (juego->getIndiceNivel() == 0)
        return escenaNivel1;
    return escenaNivel2;
}

void VentanaPrincipal::alternarPruebaFisica()
{
    if (escenaActual == escenaPrueba)
        mostrarEscena(escenaDelNivel());
    else if (escenaActual == escenaNivel1 || escenaActual == escenaNivel2)
        mostrarEscena(escenaPrueba);
}

// ------------------------------------------------------------ Flujo del juego

void VentanaPrincipal::irAlMenu()
{
    escenaMenu->setHayPartida(Partida::existe(rutaPartida));
    mostrarEscena(escenaMenu);
}

void VentanaPrincipal::nuevaPartida()
{
    juego->nuevaPartida();
    escenaNivel1->reconstruir();
    mostrarEscena(escenaNivel1);
}

void VentanaPrincipal::continuarPartida()
{
    if (!Partida::cargar(*juego, rutaPartida)) {
        escenaMenu->setHayPartida(false);
        escenaMenu->mostrarAviso("No se pudo leer la partida guardada");
        return;
    }
    if (juego->getIndiceNivel() == 0)
        escenaNivel1->reconstruir();
    else
        escenaNivel2->reconstruir();
    mostrarEscena(escenaDelNivel());
    escenaActual->mostrarAviso("Partida cargada");
}

void VentanaPrincipal::guardarPartida()
{
    bool ok = Partida::guardar(*juego, rutaPartida);
    if (escenaActual != nullptr)
        escenaActual->mostrarAviso(ok ? "Partida guardada" : "No se pudo guardar la partida");
}

// Enter con el nivel superado (pasa al rio) o con el juego terminado (final)
void VentanaPrincipal::siguienteNivel()
{
    if (juego->getEstado() == EstadoJuego::NIVEL_SUPERADO) {
        juego->pasarAlSiguienteNivel();
        escenaNivel2->reconstruir();
        mostrarEscena(escenaNivel2);
        // Se guarda solo al llegar a un nivel nuevo
        if (Partida::guardar(*juego, rutaPartida))
            escenaNivel2->mostrarAviso("Partida guardada");
    } else if (juego->getEstado() == EstadoJuego::FINAL) {
        Partida::borrar(rutaPartida);   // la historia termino: no hay nada que continuar
        escenaFinal->comenzar();
        mostrarEscena(escenaFinal);
    }
}
