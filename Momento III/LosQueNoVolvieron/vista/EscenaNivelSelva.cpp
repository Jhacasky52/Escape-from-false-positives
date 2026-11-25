#include "EscenaNivelSelva.h"

#include "control/Juego.h"
#include "niveles/NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include "entidades/Piedra.h"
#include "rasgos/ConocedorDelMonte.h"
#include "rasgos/Miedoso.h"
#include "agente/Anima.h"
#include "agente/Aprendizaje.h"
#include "agente/Razonamiento.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#ifdef HAY_MULTIMEDIA
#include <QSoundEffect>
#endif
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>
#include <QTransform>
#include <QtMath>

namespace {
// Capas de dibujo (z): suelo < personajes < vegetacion < oscuridad < luz < interfaz
const double Z_SUELO = 0, Z_ESCONDITE = 5, Z_SOMBRAS = 8, Z_PERSONAJES = 10, Z_VEGETACION = 20,
             Z_PIEDRAS = 25, Z_OSCURIDAD = 30, Z_LUZ = 40, Z_ANIMAS = 45, Z_BARRAS = 50;
const int MAX_PIEDRAS_VISIBLES = 6;
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
      mapaConstruido(false), modoDepuracion(false), cargando(false), carga(0.0), faseLatido(0.0), pulso(0.0), esperaPaso(0.0), estabaCapturado(false),
      itemCarlos(nullptr), oscuridad(nullptr)
{
    spCarlosIdle    = cargar(":/sprites/nivel1_selva/carlos_top_idle.png");
    spCarlosWalk1   = cargar(":/sprites/nivel1_selva/carlos_top_walk1.png");
    spCarlosWalk2   = cargar(":/sprites/nivel1_selva/carlos_top_walk2.png");
    spCarlosRun     = cargar(":/sprites/nivel1_selva/carlos_top_run.png");
    spCarlosThrow   = cargar(":/sprites/nivel1_selva/carlos_top_throw.png");
    spCarlosEscondido = cargar(":/sprites/nivel1_selva/carlos_top_escondido.png");
    spAnimaBlanca = cargar(":/sprites/animas/anima_blanca_segura.png");
    spAnimaRoja   = cargar(":/sprites/animas/anima_roja_peligro.png");
    spAnimaTenue  = cargar(":/sprites/animas/anima_tenue_oculta.png");
    spAnimaRamon  = cargar(":/sprites/animas/anima_ramon.png");
    spPiedra[0] = cargar(":/sprites/nivel1_selva/piedra_altura_0.png");
    spPiedra[1] = cargar(":/sprites/nivel1_selva/piedra_altura_1.png");
    spPiedra[2] = cargar(":/sprites/nivel1_selva/piedra_altura_2.png");
    spSoldadoWalk1  = cargar(":/sprites/nivel1_selva/soldado_top_walk1.png");
    spSoldadoWalk2  = cargar(":/sprites/nivel1_selva/soldado_top_walk2.png");
    spSoldadoAlerta = cargar(":/sprites/nivel1_selva/soldado_top_persiguiendo.png");

    construirMapa();
    crearSpritesEntidades();

    // Piedras: un grupo fijo de sprites que se reutilizan
    for (int i = 0; i < MAX_PIEDRAS_VISIBLES; ++i) {
        QGraphicsEllipseItem *sombra = addEllipse(-6, -3, 12, 6, Qt::NoPen, QColor(0, 0, 0, 130));
        sombra->setZValue(Z_SOMBRAS);
        sombra->hide();
        sombrasPiedras.append(sombra);
        QGraphicsPixmapItem *p = addPixmap(spPiedra[0]);
        p->setOffset(-32, -32);
        p->setZValue(Z_PIEDRAS);
        p->hide();
        itemsPiedras.append(p);
    }

#ifdef HAY_MULTIMEDIA
    sonidoLatido = new QSoundEffect(this);
    sonidoLatido->setSource(QUrl("qrc:/sonidos/latido.wav"));
#endif
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
    qDeleteAll(signosSoldados);
    qDeleteAll(itemsAnimas);
    itemsAnimas.clear();
    itemsSoldados.clear();
    itemsConos.clear();
    barrasSospecha.clear();
    signosSoldados.clear();
    cargando = false;
    carga = 0.0;

    itemCarlos = addPixmap(spCarlosIdle);
    itemCarlos->setOffset(-64, -64);
    itemCarlos->setScale(ESCALA_PERSONAJE);
    itemCarlos->setZValue(Z_PERSONAJES + 1);

    // Las animas son luz: se dibujan por encima de la oscuridad
    for (int i = 0; i < nivel->getAnimas().size(); ++i) {
        QGraphicsPixmapItem *a = addPixmap(spAnimaBlanca);
        a->setOffset(-32, -32);
        a->setZValue(Z_ANIMAS);
        itemsAnimas.append(a);
    }

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

        QGraphicsSimpleTextItem *signo = addSimpleText("?");
        QFont f = signo->font();
        f.setPointSize(16);
        f.setBold(true);
        signo->setFont(f);
        signo->setBrush(QColor(240, 210, 80));
        signo->setZValue(Z_BARRAS);
        signo->hide();
        signosSoldados.append(signo);
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
        if (cargando)
            carga = qMin(1.0, carga + dt);   // 1 segundo para la fuerza maxima
        actualizarLatidos(dt);

        // Pasos: caminando cada 0.45 s y bajito, corriendo mas seguido y fuerte
        esperaPaso -= dt;
        if (carlos->estaMoviendose() && esperaPaso <= 0.0) {
            bool corre = carlos->estaCorriendo();
            sonar("pasos", corre ? 0.55 : 0.25);
            esperaPaso = corre ? 0.28 : 0.45;
        }
    }
    juego->actualizar(dt);

    bool capturado = juego->getEstado() == EstadoJuego::CAPTURADO;
    if (capturado && !estabaCapturado)
        sonar("captura", 0.9);
    estabaCapturado = capturado;
}

void EscenaNivelSelva::actualizarLatidos(double dt)
{
    Carlos *carlos = nivel->getCarlos();
    pulso = qMax(0.0, pulso - dt * 4.0);
    faseLatido += dt * carlos->ritmoLatidos() / 60.0;
    if (faseLatido >= 1.0) {
        faseLatido -= 1.0;
        pulso = 1.0;
#ifdef HAY_MULTIMEDIA
        // Con poco panico el corazon casi no se oye; aterrorizado retumba
        double panico = carlos->getPanico();
        if (panico > 10.0) {
            sonidoLatido->setVolume(0.15 + 0.85 * panico / 100.0);
            sonidoLatido->play();
        }
#endif
    }
}

void EscenaNivelSelva::teclaSoltada(int tecla)
{
    if (tecla == Qt::Key_Space && cargando) {
        cargando = false;
        if (juego->getEstado() == EstadoJuego::JUGANDO)
            nivel->lanzarPiedra(carga);
        carga = 0.0;
    }
}

void EscenaNivelSelva::teclaPulsada(int tecla)
{
    EstadoJuego estado = juego->getEstado();
    Carlos *carlos = nivel->getCarlos();
    if (tecla == Qt::Key_P || tecla == Qt::Key_Escape) {
        juego->alternarPausa();
    } else if (tecla == Qt::Key_G && estado == EstadoJuego::PAUSADO) {
        emit guardarPartida();
    } else if (tecla == Qt::Key_M && (estado == EstadoJuego::PAUSADO || estado == EstadoJuego::CAPTURADO)) {
        emit volverAlMenu();
    } else if (tecla == Qt::Key_Space && estado == EstadoJuego::JUGANDO
               && carlos != nullptr && carlos->puedeLanzar()) {
        cargando = true;
        carga = 0.0;
    } else if (tecla == Qt::Key_F1) {
        modoDepuracion = !modoDepuracion;
    } else if (tecla == Qt::Key_E && estado == EstadoJuego::JUGANDO && carlos != nullptr) {
        if (carlos->estaEscondido())
            carlos->salirDelEscondite();
        else
            carlos->esconderse();
    } else if (tecla == Qt::Key_Return || tecla == Qt::Key_Enter) {
        if (estado == EstadoJuego::NIVEL_SUPERADO) {
            emit siguienteNivel();   // VentanaPrincipal pasa al rio
        } else if (estado == EstadoJuego::CAPTURADO) {
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
    if (carlos->estaEscondido())
        sp = &spCarlosEscondido;
    else if (cargando)
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

        QGraphicsSimpleTextItem *signo = signosSoldados[i];
        if (s->getEstado() == EstadoSoldado::INVESTIGANDO) {
            signo->setPos(s->getPosicion() + QPointF(-6, -58));
            signo->show();
        } else {
            signo->hide();
        }

        QGraphicsRectItem *barra = barrasSospecha[i];
        if (s->getSospecha() > 0.01 && !persigue) {
            barra->setRect(0, 0, 40 * s->getSospecha(), 5);
            barra->setPos(s->getPosicion() + QPointF(-20, -38));
            barra->show();
        } else {
            barra->hide();
        }
    }

    actualizarPiedras();
    actualizarAnimas();

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

void EscenaNivelSelva::actualizarPiedras()
{
    const QList<Piedra *> &piedras = nivel->getPiedras();
    for (int i = 0; i < itemsPiedras.size(); ++i) {
        if (i >= piedras.size()) {
            itemsPiedras[i]->hide();
            sombrasPiedras[i]->hide();
            continue;
        }
        const Piedra *p = piedras[i];
        double z = p->getAltura();
        int indice = z < 25 ? 0 : (z < 60 ? 1 : 2);
        itemsPiedras[i]->setPixmap(spPiedra[indice]);
        itemsPiedras[i]->setScale(0.45 + z / 250.0);
        // La altura se ve como tamano y como separacion de la sombra
        itemsPiedras[i]->setPos(p->getPosicion().x(), p->getPosicion().y() - z * 0.5);
        sombrasPiedras[i]->setPos(p->getPosicion());
        sombrasPiedras[i]->setScale(1.0 + z / 120.0);
        itemsPiedras[i]->setOpacity(p->cayo() ? 0.6 : 1.0);
        itemsPiedras[i]->show();
        sombrasPiedras[i]->setVisible(!p->cayo());
    }
}

void EscenaNivelSelva::actualizarAnimas()
{
    const QList<Anima *> &animas = nivel->getAnimas();
    for (int i = 0; i < animas.size() && i < itemsAnimas.size(); ++i) {
        const Anima *a = animas[i];
        QGraphicsPixmapItem *item = itemsAnimas[i];
        const QPixmap *p = &spAnimaBlanca;
        switch (a->getColor()) {
        case ColorAnima::ROJA:  p = &spAnimaRoja; break;
        case ColorAnima::TENUE: p = &spAnimaTenue; break;
        case ColorAnima::RAMON: p = &spAnimaRamon; break;
        default: break;
        }
        if (item->pixmap().cacheKey() != p->cacheKey()) {
            item->setPixmap(*p);
            item->setOffset(-p->width() / 2.0, -p->height() / 2.0);
        }
        item->setPos(a->posicionDibujo());
        item->setOpacity(a->esVisible() ? a->getBrillo() : 0.0);
        item->setScale(0.7 + 0.15 * a->getBrillo());
    }
}

// Modo depuracion (F1): muestra el razonamiento de las animas sobre el mapa
void EscenaNivelSelva::dibujarDepuracion(QPainter *painter)
{
    const double T = NivelSelva::TAM_CELDA;
    Aprendizaje *mem = nivel->getMemoria();

    // Peligro aprendido (persiste entre intentos): cuadros rojos
    if (mem != nullptr) {
        for (auto it = mem->getMapaPeligro().cbegin(); it != mem->getMapaPeligro().cend(); ++it) {
            double v = mem->peligroAprendido(it.key());
            painter->fillRect(QRectF(it.key().x() * T, it.key().y() * T, T, T),
                              QColor(220, 30, 30, int(40 + 120 * v)));
        }
    }
    // Riesgo que calculo la primera anima en su ultima decision: verde bajo, rojo alto
    if (!nivel->getAnimas().isEmpty()) {
        const QMap<QPoint, double> &riesgo = nivel->getAnimas().first()->getRazonamiento()->getUltimoRiesgo();
        QFont f = painter->font();
        f.setPointSize(8);
        painter->setFont(f);
        for (auto it = riesgo.cbegin(); it != riesgo.cend(); ++it) {
            double r = qBound(-0.5, it.value(), 2.0);
            double t = (r + 0.5) / 2.5;   // 0..1
            QRectF celda(it.key().x() * T + 6, it.key().y() * T + 6, T - 12, T - 12);
            painter->setPen(QPen(QColor(int(255 * t), int(255 * (1 - t)), 60, 220), 2));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(celda);
            painter->drawText(celda, Qt::AlignCenter, QString::number(it.value(), 'f', 2));
        }
    }
    // Linea de cada anima a su destino
    const char *nombres[] = {"GUIAR", "ADVERTIR", "OCULTARSE", "CALMAR"};
    for (const Anima *a : nivel->getAnimas()) {
        const Decision &d = a->getDecision();
        painter->setPen(QPen(QColor(200, 220, 255, 200), 1, Qt::DashLine));
        painter->drawLine(a->getPosicion(), d.destino);
        painter->setPen(QColor(230, 240, 255));
        painter->drawText(a->getPosicion() + QPointF(14, -10), nombres[static_cast<int>(d.tipo)]);
    }
}

// Interfaz dibujada encima de todo, en coordenadas de la pantalla
void EscenaNivelSelva::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    if (views().isEmpty())
        return;
    QWidget *pantalla = views().first()->viewport();
    const int w = pantalla->width(), h = pantalla->height();

    // En coordenadas del mundo: modo depuracion
    if (modoDepuracion) {
        painter->save();
        dibujarDepuracion(painter);
        painter->restore();
    }

    painter->save();
    painter->resetTransform();

    QFont f = painter->font();
    f.setPointSize(11);
    painter->setFont(f);
    painter->setPen(QColor(220, 220, 200));
    painter->drawText(12, 22, "Nivel 1: La selva   |   llega al rio (derecha) sin que te atrapen");
    painter->drawText(12, h - 12, "WASD mover  Shift correr  Espacio (mantener) lanzar  E esconderse  P pausa  F1 animas  F9 fisicas");
    if (modoDepuracion && nivel->getMemoria() != nullptr) {
        Aprendizaje *m = nivel->getMemoria();
        painter->drawText(12, 44, QString("Animas: obediencia %1  insistencia %2  capturas recordadas %3")
                                      .arg(m->getTasaObediencia(), 0, 'f', 2)
                                      .arg(m->insistencia(), 0, 'f', 2)
                                      .arg(m->numeroCapturas()));
    }

    Carlos *carlos = nivel->getCarlos();
    if (carlos != nullptr) {
        // Corazon que late al ritmo del panico + barra de panico
        double panico = carlos->getPanico();
        QColor rojo(200 + 55 * pulso, 40, 50);
        double radio = 9 + 4 * pulso;
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setBrush(rojo);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(w - 230, 20), radio, radio);
        painter->setBrush(QColor(255, 255, 255, 40));
        painter->drawRect(w - 212, 14, 150, 12);
        QColor colorBarra = panico > 60 ? QColor(220, 60, 50) : QColor(220, 170, 60);
        painter->setBrush(colorBarra);
        painter->drawRect(QRectF(w - 212, 14, 1.5 * panico, 12));
        painter->setPen(QColor(220, 220, 200));
        painter->drawText(w - 55, 25, QString::number(int(carlos->ritmoLatidos())));

        // Escondite: tiempo que le queda
        if (carlos->estaEscondido()) {
            double maximo = 6.0;
            for (Rasgo *r : carlos->getRasgos())
                if (auto *c = dynamic_cast<ConocedorDelMonte *>(r))
                    maximo = c->getTiempoMaxEscondido();
            double resto = qMax(0.0, 1.0 - carlos->getTiempoEscondido() / maximo);
            painter->setPen(QColor(200, 230, 200));
            painter->drawText(QRect(0, h - 70, w, 20), Qt::AlignHCenter, "Escondido (E para salir)");
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(255, 255, 255, 40));
            painter->drawRect(w / 2 - 75, h - 48, 150, 8);
            painter->setBrush(QColor(120, 200, 120));
            painter->drawRect(QRectF(w / 2 - 75, h - 48, 150 * resto, 8));
        } else if (nivel->esEscondite(carlos->getPosicion())) {
            painter->setPen(QColor(200, 230, 200));
            painter->drawText(QRect(0, h - 70, w, 20), Qt::AlignHCenter, "E: esconderse");
        }

        // Fuerza del lanzamiento
        if (cargando) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(QColor(255, 255, 255, 40));
            painter->drawRect(w / 2 - 75, h / 2 + 46, 150, 8);
            painter->setBrush(QColor(230, 220, 160));
            painter->drawRect(QRectF(w / 2 - 75, h / 2 + 46, 150 * carga, 8));
        }

        // Con mucho panico la pantalla se oscurece en los bordes al ritmo del corazon
        if (panico > 60) {
            QRadialGradient v(QPointF(w / 2.0, h / 2.0), w * 0.7);
            v.setColorAt(0.6, QColor(60, 0, 0, 0));
            v.setColorAt(1.0, QColor(60, 0, 0, int((panico - 60) * 3 * (0.6 + 0.4 * pulso))));
            painter->fillRect(0, 0, w, h, v);
        }
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

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
        mensaje = "Te atraparon.\n\nEnter: intentarlo de nuevo     M: menu principal";
        break;
    case EstadoJuego::PAUSADO:
        mensaje = "Pausa\n\nP: continuar     G: guardar partida     M: menu principal";
        break;
    case EstadoJuego::NIVEL_SUPERADO:
    case EstadoJuego::FINAL:
        mensaje = "Llegaste al rio.\n\nAhora hay que cruzarlo sin que te vean.\nPresiona Enter para continuar";
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
    dibujarAviso(painter, w, h);
    painter->restore();
}
