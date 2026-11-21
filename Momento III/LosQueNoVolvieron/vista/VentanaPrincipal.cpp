#include "VentanaPrincipal.h"
#include "EscenaNivelSelva.h"
#include "EscenaPruebaFisica.h"
#include "control/Juego.h"
#include "niveles/NivelSelva.h"

#include <QGraphicsView>
#include <QShortcut>

VentanaPrincipal::VentanaPrincipal(QWidget *parent)
    : QMainWindow(parent), juego(new Juego()), escenaActual(nullptr)
{
    setWindowTitle("Los que no volvieron");

    // Logica: el juego con sus niveles (el nivel 2 se agrega en la fase 5)
    NivelSelva *selva = new NivelSelva();
    juego->agregarNivel(selva);
    juego->iniciar();

    // Graficos: una escena por nivel + la escena de prueba de fisicas (F9)
    escenaNivel1 = new EscenaNivelSelva(juego, selva, this);
    escenaPrueba = new EscenaPruebaFisica(this);
    escenaPrueba->setSceneRect(0, 0, ANCHO, ALTO);

    vista = new QGraphicsView(this);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, false);   // pixel art nitido
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    vista->setFixedSize(ANCHO + 2, ALTO + 2);
    vista->setBackgroundBrush(QColor(8, 10, 8));
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
    escenaPrueba->detenerBucle();
    delete escenaNivel1;
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
    mostrarEscena(escenaActual == escenaNivel1 ? escenaPrueba : escenaNivel1);
}
