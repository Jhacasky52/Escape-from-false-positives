#include "EscenaPruebaFisica.h"

#include "entidades/Carlos.h"
#include "fisica/MovimientoParabolico.h"
#include "fisica/MovimientoPendular.h"
#include "fisica/MovimientoFluido.h"
#include "fisica/MovimientoOscilatorio.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QtMath>

namespace {
// Zonas de la pantalla de prueba
const QRectF ZONA_SELVA(0, 0, 640, 400);
const QRectF ZONA_RIO(0, 400, 960, 200);
const double NIVEL_AGUA = 470.0;      // y de la superficie del agua
const double ESCALA_PERSONAJE = 0.5;  // los sprites de 128 px se ven de 64 px

QPixmap cargar(const QString &ruta)
{
    QPixmap p(ruta);
    if (p.isNull())
        qWarning("No se encontro el sprite %s", qPrintable(ruta));
    return p;
}

QGraphicsSimpleTextItem *crearTexto(QGraphicsScene *escena, const QString &texto,
                                    const QPointF &pos, int tamano = 10)
{
    QGraphicsSimpleTextItem *t = escena->addSimpleText(texto);
    QFont f("Consolas");
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(tamano);
    t->setFont(f);
    t->setBrush(QColor(225, 225, 210));
    t->setPos(pos);
    t->setZValue(100);
    return t;
}
}

EscenaPruebaFisica::EscenaPruebaFisica(QObject *parent)
    : EscenaJuego(parent),
      carlos(new Carlos(QPointF(160, 300))),
      tiro(nullptr), cargando(false), carga(0.0),
      pendulo(new MovimientoPendular(qDegreesToRadians(35.0), 4.0)),
      posSoldado(470, 90),
      fluido(new MovimientoFluido(1.0, 2.5, 0.8, 120.0)),
      origenCuerpo(120, 462),
      oleaje(new MovimientoOscilatorio(6.0, 2.2)),
      xTronco(-200.0),
      tiempoAnimacion(0.0)
{
    carlos->setLimites(ZONA_SELVA.adjusted(24, 24, -24, -24));
    posCuerpo = origenCuerpo;

    construirSelva();
    construirRio();
    construirPanel();
}

EscenaPruebaFisica::~EscenaPruebaFisica()
{
    delete carlos;
    delete tiro;
    delete pendulo;
    delete fluido;
    delete oleaje;
    qDeleteAll(vaivenes);
}

// ------------------------------------------------------------ Construccion

void EscenaPruebaFisica::construirSelva()
{
    // Suelo con tiles y una trocha vertical
    QPixmap suelo = cargar(":/sprites/nivel1_selva/tile_suelo_selva.png");
    QPixmap trocha = cargar(":/sprites/nivel1_selva/tile_trocha.png");
    for (int x = 0; x < ZONA_SELVA.width(); x += 64) {
        for (int y = 0; y < ZONA_SELVA.height(); y += 64) {
            const QPixmap &tile = (x == 128) ? trocha : suelo;
            QGraphicsPixmapItem *it = addPixmap(tile.scaled(64, 64));
            it->setPos(x, y);
            it->setZValue(0);
        }
    }

    QGraphicsPixmapItem *arbol = addPixmap(cargar(":/sprites/nivel1_selva/arbol_copa.png"));
    arbol->setPos(20, 20);
    arbol->setScale(0.6);
    arbol->setZValue(20);

    QGraphicsPixmapItem *mata = addPixmap(cargar(":/sprites/nivel1_selva/matorral.png"));
    mata->setPos(300, 300);
    mata->setScale(0.6);
    mata->setZValue(5);

    // Carlos
    spCarlosIdle  = cargar(":/sprites/nivel1_selva/carlos_top_idle.png");
    spCarlosWalk1 = cargar(":/sprites/nivel1_selva/carlos_top_walk1.png");
    spCarlosWalk2 = cargar(":/sprites/nivel1_selva/carlos_top_walk2.png");
    spCarlosRun   = cargar(":/sprites/nivel1_selva/carlos_top_run.png");
    spCarlosThrow = cargar(":/sprites/nivel1_selva/carlos_top_throw.png");
    itemCarlos = addPixmap(spCarlosIdle);
    itemCarlos->setOffset(-64, -64);   // el origen del item queda en el centro
    itemCarlos->setScale(ESCALA_PERSONAJE);
    itemCarlos->setZValue(10);

    // Piedra y su sombra
    spPiedra[0] = cargar(":/sprites/nivel1_selva/piedra_altura_0.png");
    spPiedra[1] = cargar(":/sprites/nivel1_selva/piedra_altura_1.png");
    spPiedra[2] = cargar(":/sprites/nivel1_selva/piedra_altura_2.png");
    sombraPiedra = addEllipse(-6, -3, 12, 6, Qt::NoPen, QColor(0, 0, 0, 120));
    sombraPiedra->setZValue(9);
    sombraPiedra->hide();
    itemPiedra = addPixmap(spPiedra[0]);
    itemPiedra->setOffset(-32, -32);
    itemPiedra->setZValue(30);
    itemPiedra->hide();

    // Soldado con la linterna (cono). Los sprites miran hacia arriba;
    // este soldado mira hacia abajo, por eso se gira 180 grados.
    itemSoldado = addPixmap(cargar(":/sprites/nivel1_selva/soldado_top_walk1.png"));
    itemSoldado->setOffset(-64, -64);
    itemSoldado->setScale(ESCALA_PERSONAJE);
    itemSoldado->setRotation(180);
    itemSoldado->setPos(posSoldado);
    itemSoldado->setZValue(11);

    itemCono = addPixmap(cargar(":/sprites/nivel1_selva/cono_linterna.png"));
    itemCono->setOffset(-96, -256);    // el vertice del cono queda en el origen
    itemCono->setPos(posSoldado);
    itemCono->setScale(0.8);
    itemCono->setOpacity(0.55);
    itemCono->setZValue(12);

    crearTexto(this, "SELVA: WASD/flechas mover, Shift correr, mantener Espacio y soltar = lanzar piedra",
               QPointF(8, 4), 9);
}

void EscenaPruebaFisica::construirRio()
{
    QGraphicsRectItem *cielo = addRect(ZONA_RIO, Qt::NoPen, QColor(14, 18, 34));
    cielo->setZValue(0);
    QGraphicsRectItem *agua = addRect(QRectF(0, NIVEL_AGUA, 960, 600 - NIVEL_AGUA),
                                      Qt::NoPen, QColor(30, 70, 110, 220));
    agua->setZValue(15);

    itemTronco = addPixmap(cargar(":/sprites/nivel2_rio/tronco_flotante.png"));
    itemTronco->setOffset(-96, -32);
    itemTronco->setScale(0.7);
    itemTronco->setZValue(16);

    itemCuerpo = addPixmap(cargar(":/sprites/nivel2_rio/carlos_lado_arrastrado.png"));
    itemCuerpo->setOffset(-48, -80);
    itemCuerpo->setScale(0.5);
    itemCuerpo->setZValue(16);   // el sprite ya trae dibujada el agua

    crearTexto(this, "RIO: la corriente arrastra a Carlos; mantener J = nadar contra la corriente, R = reiniciar",
               QPointF(8, 404), 9);
}

void EscenaPruebaFisica::construirPanel()
{
    QGraphicsRectItem *fondo = addRect(QRectF(640, 0, 320, 400), Qt::NoPen, QColor(22, 24, 30));
    fondo->setZValue(1);

    const QStringList animas = {
        ":/sprites/animas/anima_blanca_segura.png",
        ":/sprites/animas/anima_roja_peligro.png",
        ":/sprites/animas/anima_tenue_oculta.png",
        ":/sprites/animas/anima_ramon.png"
    };
    for (int i = 0; i < animas.size(); ++i) {
        QGraphicsPixmapItem *it = addPixmap(cargar(animas[i]));
        QPixmap p = it->pixmap();
        it->setOffset(-p.width() / 2.0, -p.height() / 2.0);
        it->setZValue(20);
        QPointF base(690 + i * 72, 60);
        it->setPos(base);
        itemsAnimas.append(it);
        basesAnimas.append(base);
        // Cada anima con su propia fase para que no floten sincronizadas
        vaivenes.append(new MovimientoOscilatorio(8.0, 2.0 + 0.3 * i, i * 1.3));
    }

    textoInfo = crearTexto(this, "", QPointF(652, 110), 10);
    textoBarraCarga = crearTexto(this, "", QPointF(8, 380), 10);
}

// ------------------------------------------------------------ Logica

void EscenaPruebaFisica::actualizarLogica(double dt)
{
    tiempoAnimacion += dt;

    // --- Carlos ---
    QPointF dir;
    if (teclaPresionada(Qt::Key_A) || teclaPresionada(Qt::Key_Left))  dir.rx() -= 1;
    if (teclaPresionada(Qt::Key_D) || teclaPresionada(Qt::Key_Right)) dir.rx() += 1;
    if (teclaPresionada(Qt::Key_W) || teclaPresionada(Qt::Key_Up))    dir.ry() -= 1;
    if (teclaPresionada(Qt::Key_S) || teclaPresionada(Qt::Key_Down))  dir.ry() += 1;
    carlos->setDireccion(dir);
    carlos->correr(teclaPresionada(Qt::Key_Shift));
    carlos->actualizar(dt);

    // --- Carga del lanzamiento: 1 segundo para llegar al maximo ---
    if (cargando)
        carga = qMin(1.0, carga + dt);

    // --- Piedra en el aire (tiro parabolico) ---
    if (tiro != nullptr) {
        tiro->paso(dt);
        if (tiro->aterrizo()) {
            // Fase 3: aqui la piedra hace ruido y los soldados la escuchan
            delete tiro;
            tiro = nullptr;
        }
    }

    // --- Linterna (pendulo) ---
    pendulo->paso(dt);

    // --- Cuerpo en el rio (fluido con arrastre) ---
    fluido->setFuerzaExterna(teclaPresionada(Qt::Key_J) ? QPointF(-380.0, 0.0) : QPointF());
    posCuerpo = origenCuerpo + fluido->paso(dt);
    if (posCuerpo.x() > 1000 || posCuerpo.x() < -40)
        reiniciarCuerpoRio();

    // --- Tronco (oscilatorio) que ademas baja con la corriente ---
    oleaje->paso(dt);
    xTronco += 0.6 * fluido->getVelocidadRio() * dt;
    if (xTronco > 1100)
        xTronco = -150;

    for (MovimientoOscilatorio *v : vaivenes)
        v->paso(dt);
}

void EscenaPruebaFisica::lanzarPiedra()
{
    if (tiro != nullptr)
        return;   // una piedra a la vez
    double v0 = 180.0 + 320.0 * carga;                 // fuerza segun la carga
    tiro = new MovimientoParabolico(v0, qDegreesToRadians(45.0));
    origenTiro = carlos->getPosicion();
    double th = carlos->getOrientacion();
    direccionTiro = QPointF(qCos(th), qSin(th));
}

void EscenaPruebaFisica::reiniciarCuerpoRio()
{
    fluido->reiniciar();
    posCuerpo = origenCuerpo;
}

void EscenaPruebaFisica::teclaPulsada(int tecla)
{
    if (tecla == Qt::Key_Space && tiro == nullptr) {
        cargando = true;
        carga = 0.0;
    } else if (tecla == Qt::Key_R) {
        reiniciarCuerpoRio();
    }
}

void EscenaPruebaFisica::teclaSoltada(int tecla)
{
    if (tecla == Qt::Key_Space && cargando) {
        lanzarPiedra();
        cargando = false;
    }
}

// ------------------------------------------------------------ Vista

void EscenaPruebaFisica::actualizarVista()
{
    // --- Carlos: sprite segun lo que hace ---
    const QPixmap *sp = &spCarlosIdle;
    if (cargando)
        sp = &spCarlosThrow;
    else if (carlos->estaMoviendose()) {
        if (carlos->estaCorriendo())
            sp = &spCarlosRun;
        else
            sp = (static_cast<int>(tiempoAnimacion / 0.18) % 2 == 0) ? &spCarlosWalk1 : &spCarlosWalk2;
    }
    if (itemCarlos->pixmap().cacheKey() != sp->cacheKey())
        itemCarlos->setPixmap(*sp);
    itemCarlos->setPos(carlos->getPosicion());
    // Los sprites miran hacia arriba (-90 grados), por eso se suma 90
    itemCarlos->setRotation(qRadiansToDegrees(carlos->getOrientacion()) + 90.0);

    // --- Piedra: la altura z se ve como tamano y la sombra queda en el suelo ---
    if (tiro != nullptr) {
        QPointF suelo = origenTiro + direccionTiro * tiro->distancia();
        double z = tiro->altura();
        int indice = z < 25 ? 0 : (z < 60 ? 1 : 2);
        itemPiedra->setPixmap(spPiedra[indice]);
        itemPiedra->setScale(0.45 + z / 250.0);
        itemPiedra->setPos(suelo.x(), suelo.y() - z * 0.5);   // un poco hacia arriba = altura
        sombraPiedra->setPos(suelo);
        sombraPiedra->setScale(1.0 + z / 120.0);
        itemPiedra->show();
        sombraPiedra->show();
    } else {
        itemPiedra->hide();
        sombraPiedra->hide();
    }

    // --- Cono de la linterna: apunta hacia abajo (180) mas el angulo del pendulo ---
    itemCono->setRotation(180.0 + qRadiansToDegrees(pendulo->anguloActual()));

    // --- Rio ---
    itemCuerpo->setPos(posCuerpo);
    itemTronco->setPos(xTronco, NIVEL_AGUA - 4 + oleaje->paso(0).y());

    // --- Animas flotando ---
    for (int i = 0; i < itemsAnimas.size(); ++i)
        itemsAnimas[i]->setPos(basesAnimas[i] + vaivenes[i]->paso(0));

    // --- Panel de datos ---
    QPointF v = fluido->getVelocidad();
    QPointF vt = fluido->velocidadTerminal();
    QString info;
    info += QString("FPS: %1\n\n").arg(getFps(), 0, 'f', 0);
    info += "TIRO PARABOLICO (piedra)\n";
    if (tiro != nullptr)
        info += QString(" x = %1 px   z = %2 px\n t = %3 / %4 s\n")
                    .arg(tiro->distancia(), 0, 'f', 0).arg(tiro->altura(), 0, 'f', 0)
                    .arg(tiro->getTiempo(), 0, 'f', 2).arg(tiro->tiempoDeVuelo(), 0, 'f', 2);
    else
        info += " (sin piedra en el aire)\n\n";
    info += "\nPENDULO (linterna)\n";
    info += QString(" theta = %1 grados\n T = %2 s\n")
                .arg(qRadiansToDegrees(pendulo->anguloActual()), 0, 'f', 1)
                .arg(pendulo->periodo(), 0, 'f', 2);
    info += "\nFLUIDO (Carlos en el rio)\n";
    info += QString(" v = %1 px/s\n v terminal = %2 px/s\n")
                .arg(v.x(), 0, 'f', 1).arg(vt.x(), 0, 'f', 1);
    info += "\nOSCILATORIO (tronco y animas)\n";
    info += QString(" y = %1 px\n").arg(oleaje->paso(0).y(), 0, 'f', 1);
    textoInfo->setText(info);

    if (cargando)
        textoBarraCarga->setText("Fuerza: [" + QString(int(carga * 20), '#')
                                 + QString(20 - int(carga * 20), '.') + "]");
    else
        textoBarraCarga->setText("");
}
