#include "EscenaNivelSelva.h"

#include "control/Juego.h"
#include "niveles/NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>
#include <QTransform>
#include <QtMath>

namespace {
// Capas de dibujo (z): suelo < personajes < vegetacion < oscuridad < luz < interfaz
const double Z_SUELO = 0, Z_ESCONDITE = 5, Z_PERSONAJES = 10, Z_VEGETACION = 20,
             Z_OSCURIDAD = 30, Z_LUZ = 40, Z_BARRAS = 50;
const double ESCALA_PERSONAJE = 0.5;   // sprites de 128 px -> 64 px

QPixmap cargar(const QString &ruta)
{
    QPixmap p(ruta);
    if (p.isNull())
        qWarning("No se encontro el sprite %s", qPrintable(ruta));
    return p;
}

double grados(double radianes) { return qRadiansToDegrees(radianes); }
}

EscenaNivelSelva::EscenaNivelSelva(Juego *juego, NivelSelva *nivel, QObject *parent)
    : EscenaJuego(parent), juego(juego), nivel(nivel), tiempoAnimacion(0.0),
      mapaConstruido(false), itemCarlos(nullptr), oscuridad(nullptr)
{
    spCarlosIdle    = cargar(":/sprites/nivel1_selva/carlos_top_idle.png");
    spCarlosWalk1   = cargar(":/sprites/nivel1_selva/carlos_top_walk1.png");
    spCarlosWalk2   = cargar(":/sprites/nivel1_selva/carlos_top_walk2.png");
    spCarlosRun     = cargar(":/sprites/nivel1_selva/carlos_top_run.png");
    spSoldadoWalk1  = cargar(":/sprites/nivel1_selva/soldado_top_walk1.png");
    spSoldadoWalk2  = cargar(":/sprites/nivel1_selva/soldado_top_walk2.png");
    spSoldadoAlerta = cargar(":/sprites/nivel1_selva/soldado_top_persiguiendo.png");

    construirMapa();
    crearSpritesEntidades();
}

// ------------------------------------------------------------ Construccion

void EscenaNivelSelva::construirMapa()
{
    const int T = NivelSelva::TAM_CELDA;
    setSceneRect(nivel->getLimites());

    QPixmap suelo   = cargar(":/sprites/nivel1_selva/tile_suelo_selva.png");
    QPixmap trocha  = cargar(":/sprites/nivel1_selva/tile_trocha.png");
    // El tile de trocha es vertical; para los tramos horizontales se gira 90 grados
    QPixmap trochaHorizontal = trocha.transformed(QTransform().rotate(90));
    QPixmap arbol   = cargar(":/sprites/nivel1_selva/arbol_copa.png");
    QPixmap mata    = cargar(":/sprites/nivel1_selva/matorral.png");
    QPixmap tronco  = cargar(":/sprites/nivel1_selva/tronco_hueco_escondite.png");

    for (int f = 0; f < nivel->getFilas(); ++f) {
        for (int c = 0; c < nivel->getColumnas(); ++c) {
            Celda celda = nivel->celdaEn(c, f);
            QPointF esquina(c * T, f * T);
            QPointF centro(esquina.x() + T / 2.0, esquina.y() + T / 2.0);

            if (celda == Celda::RIO) {
                QGraphicsRectItem *agua = addRect(QRectF(esquina, QSizeF(T, T)), Qt::NoPen,
                                                  QColor(28, 62, 96));
                agua->setZValue(Z_SUELO);
                continue;
            }

            const QPixmap *tile = &suelo;
            if (celda == Celda::TROCHA) {
                bool vecinoVertical = nivel->celdaEn(c, f - 1) == Celda::TROCHA
                                   || nivel->celdaEn(c, f + 1) == Celda::TROCHA;
                tile = vecinoVertical ? &trocha : &trochaHorizontal;
            }
            QGraphicsPixmapItem *base = addPixmap(*tile);
            base->setPos(esquina);
            base->setScale(T / 128.0);
            base->setZValue(Z_SUELO);

            if (celda == Celda::ARBOL) {
                QGraphicsPixmapItem *copa = addPixmap(arbol);
                copa->setOffset(-arbol.width() / 2.0, -arbol.height() / 2.0);
                copa->setScale(0.6);
                copa->setPos(centro);
                copa->setZValue(Z_VEGETACION + 1);
            } else if (celda == Celda::MATORRAL) {
                QGraphicsPixmapItem *m = addPixmap(mata);
                m->setOffset(-mata.width() / 2.0, -mata.height() / 2.0);
                m->setScale(0.6);
                m->setPos(centro);
                m->setOpacity(0.85);
                m->setZValue(Z_VEGETACION);
            } else if (celda == Celda::ESCONDITE) {
                QGraphicsPixmapItem *h = addPixmap(tronco);
                h->setOffset(-tronco.width() / 2.0, -tronco.height() / 2.0);
                h->setScale(0.5);
                h->setPos(centro);
                h->setZValue(Z_ESCONDITE);
            }
        }
    }

    // Orilla del rio: una linea clara donde empieza el agua
    QRectF meta = nivel->getMeta();
    QGraphicsRectItem *orilla = addRect(QRectF(meta.left() - 3, 0, 6, meta.height()),
                                        Qt::NoPen, QColor(150, 170, 190, 160));
    orilla->setZValue(Z_SUELO + 1);

    // Capa de oscuridad: es de noche y solo se ve cerca de Carlos
    oscuridad = addRect(nivel->getLimites(), Qt::NoPen);
    oscuridad->setZValue(Z_OSCURIDAD);
    mapaConstruido = true;
}

void EscenaNivelSelva::crearSpritesEntidades()
{
    // Se borran los sprites anteriores (al reiniciar el nivel hay entidades nuevas)
    delete itemCarlos;
    qDeleteAll(itemsSoldados);
    qDeleteAll(itemsConos);
    qDeleteAll(barrasSospecha);
    itemsSoldados.clear();
    itemsConos.clear();
    barrasSospecha.clear();

    itemCarlos = addPixmap(spCarlosIdle);
    itemCarlos->setOffset(-64, -64);
    itemCarlos->setScale(ESCALA_PERSONAJE);
    itemCarlos->setZValue(Z_PERSONAJES + 1);

    QPixmap cono = cargar(":/sprites/nivel1_selva/cono_linterna.png");
    for (int i = 0; i < nivel->getSoldados().size(); ++i) {
        QGraphicsPixmapItem *s = addPixmap(spSoldadoWalk1);
        s->setOffset(-64, -64);
        s->setScale(ESCALA_PERSONAJE);
        s->setZValue(Z_PERSONAJES);
        itemsSoldados.append(s);

        // El cono mide 256 px de largo; se escala al alcance de la linterna
        QGraphicsPixmapItem *c = addPixmap(cono);
        c->setOffset(-cono.width() / 2.0, -cono.height());   // vertice en el origen
        double alcance = nivel->getSoldados()[i]->getLinterna()->getAlcance();
        c->setScale(alcance / cono.height());
        c->setOpacity(0.5);
        c->setZValue(Z_LUZ);
        itemsConos.append(c);

        QGraphicsRectItem *barra = addRect(QRectF(0, 0, 40, 5), Qt::NoPen, QColor(230, 200, 60));
        barra->setZValue(Z_BARRAS);
        barrasSospecha.append(barra);
    }
}

// ------------------------------------------------------------ Bucle

void EscenaNivelSelva::actualizarLogica(double dt)
{
    tiempoAnimacion += dt;
    Carlos *carlos = nivel->getCarlos();
    if (carlos != nullptr && juego->getEstado() == EstadoJuego::JUGANDO) {
        QPointF dir;
        if (teclaPresionada(Qt::Key_A) || teclaPresionada(Qt::Key_Left))  dir.rx() -= 1;
        if (teclaPresionada(Qt::Key_D) || teclaPresionada(Qt::Key_Right)) dir.rx() += 1;
        if (teclaPresionada(Qt::Key_W) || teclaPresionada(Qt::Key_Up))    dir.ry() -= 1;
        if (teclaPresionada(Qt::Key_S) || teclaPresionada(Qt::Key_Down))  dir.ry() += 1;
        carlos->setDireccion(dir);
        carlos->correr(teclaPresionada(Qt::Key_Shift));
    }
    juego->actualizar(dt);
}

void EscenaNivelSelva::teclaPulsada(int tecla)
{
    EstadoJuego estado = juego->getEstado();
    if (tecla == Qt::Key_P || tecla == Qt::Key_Escape) {
        juego->alternarPausa();
    } else if (tecla == Qt::Key_Return || tecla == Qt::Key_Enter) {
        if (estado == EstadoJuego::CAPTURADO || estado == EstadoJuego::FINAL
            || estado == EstadoJuego::NIVEL_SUPERADO) {
            // Fase 5: en NIVEL_SUPERADO se pasara al rio con pasarAlSiguienteNivel()
            juego->reiniciarNivel();
            crearSpritesEntidades();
        }
    }
}

void EscenaNivelSelva::actualizarVista()
{
    Carlos *carlos = nivel->getCarlos();
    if (carlos == nullptr)
        return;

    // --- Carlos ---
    const QPixmap *sp = &spCarlosIdle;
    if (carlos->estaMoviendose()) {
        if (carlos->estaCorriendo())
            sp = &spCarlosRun;
        else
            sp = (static_cast<int>(tiempoAnimacion / 0.18) % 2 == 0) ? &spCarlosWalk1 : &spCarlosWalk2;
    }
    if (itemCarlos->pixmap().cacheKey() != sp->cacheKey())
        itemCarlos->setPixmap(*sp);
    itemCarlos->setPos(carlos->getPosicion());
    itemCarlos->setRotation(grados(carlos->getOrientacion()) + 90.0);   // los sprites miran arriba

    // --- Soldados y linternas ---
    const QList<Soldado *> &soldados = nivel->getSoldados();
    for (int i = 0; i < soldados.size() && i < itemsSoldados.size(); ++i) {
        const Soldado *s = soldados[i];
        bool persigue = s->getEstado() == EstadoSoldado::PERSIGUIENDO;
        const QPixmap *ps = persigue ? &spSoldadoAlerta
                          : ((static_cast<int>(tiempoAnimacion / 0.25 + i) % 2 == 0) ? &spSoldadoWalk1 : &spSoldadoWalk2);
        if (itemsSoldados[i]->pixmap().cacheKey() != ps->cacheKey())
            itemsSoldados[i]->setPixmap(*ps);
        itemsSoldados[i]->setPos(s->getPosicion());
        itemsSoldados[i]->setRotation(grados(s->getOrientacion()) + 90.0);

        const Linterna *l = s->getLinterna();
        itemsConos[i]->setPos(l->getOrigen());
        itemsConos[i]->setRotation(grados(l->direccionActual()) + 90.0);
        itemsConos[i]->setOpacity(persigue ? 0.7 : 0.5);

        QGraphicsRectItem *barra = barrasSospecha[i];
        if (s->getSospecha() > 0.01 && !persigue) {
            barra->setRect(0, 0, 40 * s->getSospecha(), 5);
            barra->setPos(s->getPosicion() + QPointF(-20, -38));
            barra->show();
        } else {
            barra->hide();
        }
    }

    // --- Oscuridad: transparente alrededor de Carlos, negra lejos ---
    QRadialGradient g(carlos->getPosicion(), 330);
    g.setColorAt(0.0, QColor(0, 0, 8, 10));
    g.setColorAt(0.35, QColor(0, 0, 8, 70));
    g.setColorAt(1.0, QColor(0, 0, 8, 195));
    oscuridad->setBrush(g);

    // --- Camara ---
    if (!views().isEmpty())
        views().first()->centerOn(carlos->getPosicion());
}

// Interfaz dibujada encima de todo, en coordenadas de la pantalla
void EscenaNivelSelva::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    if (views().isEmpty())
        return;
    QWidget *pantalla = views().first()->viewport();
    const int w = pantalla->width(), h = pantalla->height();

    painter->save();
    painter->resetTransform();

    QFont f = painter->font();
    f.setPointSize(11);
    painter->setFont(f);
    painter->setPen(QColor(220, 220, 200));
    painter->drawText(12, 22, "Nivel 1: La selva   |   llega al rio (derecha) sin que te atrapen");
    painter->drawText(12, h - 12, "WASD mover  Shift correr  P pausa  F9 prueba de fisicas");

    if (nivel->algunSoldadoPersigue() && juego->getEstado() == EstadoJuego::JUGANDO) {
        painter->fillRect(0, 0, w, h, QColor(140, 0, 0, 45));
        f.setPointSize(16);
        f.setBold(true);
        painter->setFont(f);
        painter->setPen(QColor(255, 80, 70));
        painter->drawText(QRect(0, 30, w, 30), Qt::AlignHCenter, "¡TE VIERON! ¡CORRE!");
    }

    QString mensaje;
    switch (juego->getEstado()) {
    case EstadoJuego::CAPTURADO:
        mensaje = "Te atraparon.\n\nPresiona Enter para intentarlo de nuevo";
        break;
    case EstadoJuego::PAUSADO:
        mensaje = "Pausa\n\nPresiona P para continuar";
        break;
    case EstadoJuego::NIVEL_SUPERADO:
    case EstadoJuego::FINAL:
        mensaje = "Llegaste al rio.\n\n(El nivel 2 llega en la fase 5)\nPresiona Enter para repetir";
        break;
    default:
        break;
    }
    if (!mensaje.isEmpty()) {
        painter->fillRect(0, 0, w, h, QColor(0, 0, 0, 170));
        f.setPointSize(18);
        f.setBold(true);
        painter->setFont(f);
        painter->setPen(QColor(235, 230, 215));
        painter->drawText(QRect(0, 0, w, h), Qt::AlignCenter, mensaje);
    }
    painter->restore();
}
