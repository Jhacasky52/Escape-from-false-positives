#include "VentanaPrincipal.h"
#include "EscenaNivelSelva.h"
#include "EscenaNivelRio.h"
#include "EscenaPruebaFisica.h"
#include "control/Juego.h"
#include "niveles/NivelSelva.h"
#include "niveles/NivelRio.h"

#include <QGraphicsView>
#include <QShortcut>

VentanaPrincipal::VentanaPrincipal(QWidget *parent)
    : QMainWindow(parent), juego(new Juego()), escenaActual(nullptr)
{
    setWindowTitle("Los que no volvieron");

    // Logica: el juego con sus dos niveles (Juego es el dueno)
    NivelSelva *selva = new NivelSelva();
    NivelRio *rio = new NivelRio();
    juego->agregarNivel(selva);
    juego->agregarNivel(rio);
    juego->iniciar();

    // Graficos: una escena por nivel + la escena de prueba de fisicas (F9)
    escenaNivel1 = new EscenaNivelSelva(juego, selva, this);
    escenaNivel2 = new EscenaNivelRio(juego, rio, this);
    connect(escenaNivel1, &EscenaJuego::siguienteNivel, this, &VentanaPrincipal::siguienteNivel);
    connect(escenaNivel2, &EscenaJuego::siguienteNivel, this, &VentanaPrincipal::siguienteNivel);
    escenaPrueba = new EscenaPruebaFisica(this);
    escenaPrueba->setSceneRect(0, 0, ANCHO, ALTO);

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

    mostrarEscena(escenaNivel1);
}

VentanaPrincipal::~VentanaPrincipal()
{
    // Las escenas se detienen antes de borrar la logica que leen
    escenaNivel1->detenerBucle();
    escenaNivel2->detenerBucle();
    escenaPrueba->detenerBucle();
    delete escenaNivel1;
    delete escenaNivel2;
    delete escenaPrueba;
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
}

void VentanaPrincipal::alternarPruebaFisica()
{
    EscenaJuego *nivel = juego->getIndiceNivel() == 0 ? escenaNivel1 : escenaNivel2;
    mostrarEscena(escenaActual == escenaPrueba ? nivel : escenaPrueba);
}

// Enter con el nivel superado: la logica carga el siguiente nivel y se cambia de escena
void VentanaPrincipal::siguienteNivel()
{
    if (juego->getEstado() == EstadoJuego::NIVEL_SUPERADO) {
        juego->pasarAlSiguienteNivel();
        static_cast<EscenaNivelRio *>(escenaNivel2)->reconstruir();
        mostrarEscena(escenaNivel2);
    }
    // EstadoJuego::FINAL: el final en Cucuta con Ramon llega en la fase 6
}
