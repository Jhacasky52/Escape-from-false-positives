#include "VentanaPrincipal.h"
#include "EscenaPruebaFisica.h"
#include "control/Juego.h"
#include <QGraphicsView>

VentanaPrincipal::VentanaPrincipal(QWidget *parent)
    : QMainWindow(parent), juego(new Juego())
{
    setWindowTitle("Los que no volvieron");

    // Fase 1: la ventana muestra la escena de prueba de las fisicas.
    // En la fase 2 se cambia por la escena del NivelSelva.
    escena = new EscenaPruebaFisica(this);
    escena->setSceneRect(0, 0, ANCHO, ALTO);

    vista = new QGraphicsView(escena, this);
    vista->setRenderHint(QPainter::SmoothPixmapTransform, false);   // pixel art nitido
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setFixedSize(ANCHO + 2, ALTO + 2);
    vista->setBackgroundBrush(QColor(12, 16, 12));
    setCentralWidget(vista);
    adjustSize();
    setFixedSize(size());

    vista->setFocus();
    escena->iniciarBucle();
}

VentanaPrincipal::~VentanaPrincipal()
{
    delete juego;
}
