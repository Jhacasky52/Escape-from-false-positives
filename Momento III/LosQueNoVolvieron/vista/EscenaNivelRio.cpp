#include "EscenaNivelRio.h"

#include "control/Juego.h"
#include "niveles/NivelRio.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include "entidades/Tronco.h"
#include "agente/Anima.h"
#include "agente/Aprendizaje.h"

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSimpleTextItem>
#ifdef HAY_MULTIMEDIA
#include <QSoundEffect>
#endif
#include <QGraphicsView>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QImage>
#include <QtMath>

namespace {
// Capas (z): rocas < personajes < agua < troncos < luz < animas < barras
const double Z_APOYOS = 5, Z_PERSONAJES = 10, Z_AGUA = 20, Z_TRONCOS = 25,
             Z_LUZ = 40, Z_ANIMAS = 45, Z_BARRAS = 50;
const int MAX_TRONCOS_VISIBLES = 10;
const double ESCALA_CARLOS = 0.5;   // sprites de 96x160 -> 48x80

QPixmap cargar(const QString &ruta)
{
    QPixmap p(ruta);
    if (p.isNull())
        qWarning("No se encontro el sprite %s", qPrintable(ruta));
    return p;
}
}

EscenaNivelRio::EscenaNivelRio(Juego *juego, NivelRio *nivel, QObject *parent)
    : EscenaJuego(parent), juego(juego), nivel(nivel), tiempoAnimacion(0.0),
      faseLatido(0.0), pulso(0.0), modoDepuracion(false), esperaChapoteo(0.0),
      estabaCapturado(false), etapaAnterior(0), itemCarlos(nullptr),
      agua(nullptr), lineaAgua(nullptr)
{
    const QString r = ":/sprites/nivel2_rio/";
    spFondo[0] = cargar(r + "fondo_rio_normal.png");
    spFondo[1] = cargar(r + "fondo_rio_media.png");
    spFondo[2] = cargar(r + "fondo_rio_creciente.png");
    // Los sprites traen el agua pintada; aqui el agua la dibuja la escena
    spCarlosNada1      = sinAgua(cargar(r + "carlos_lado_walk1.png"));
    spCarlosNada2      = sinAgua(cargar(r + "carlos_lado_walk2.png"));
    spCarlosSumergido  = sinAgua(cargar(r + "carlos_lado_sumergido.png"));
    spCarlosArrastrado = sinAgua(cargar(r + "carlos_lado_arrastrado.png"));
    spCarlosPie1 = spCarlosNada1;
    spCarlosPie2 = spCarlosNada2;
    // El soldado trae su luz dibujada; la escena dibuja el cono real de la linterna
    spSoldado = sinAgua(cargar(r + "soldado_orilla_linterna.png"));   // tambien quita la luz semitransparente
    spTronco    = cargar(r + "tronco_flotante.png");
    spEscombros = cargar(r + "escombros.png");
    // Se recorta el borde transparente de arriba para que la roca empiece donde se pisa
    spRoca      = cargar(r + "roca.png").copy(4, 16, 124, 64);
    spBanco     = cargar(r + "banco_arena.png").copy(0, 8, 192, 40);
    spAnimaBlanca = cargar(":/sprites/animas/anima_blanca_segura.png");
    spAnimaRoja   = cargar(":/sprites/animas/anima_roja_peligro.png");
    spAnimaTenue  = cargar(":/sprites/animas/anima_tenue_oculta.png");
    spAnimaRamon  = cargar(":/sprites/animas/anima_ramon.png");

    setSceneRect(0, 0, NivelRio::LARGO, NivelRio::ALTO);

    // Agua: un rectangulo semitransparente por encima de Carlos (asi se ve sumergido)
    agua = addRect(QRectF(), Qt::NoPen);
    agua->setZValue(Z_AGUA);
    lineaAgua = addRect(QRectF(), Qt::NoPen, QColor(170, 210, 235, 170));
    lineaAgua->setZValue(Z_AGUA + 1);

    for (int i = 0; i < MAX_TRONCOS_VISIBLES; ++i) {
        QGraphicsPixmapItem *t = addPixmap(spTronco);
        t->setZValue(Z_TRONCOS);
        t->hide();
        itemsTroncos.append(t);
    }

#ifdef HAY_MULTIMEDIA
    sonidoLatido = new QSoundEffect(this);
    sonidoLatido->setSource(QUrl("qrc:/sonidos/latido.wav"));
#endif
}

QPixmap EscenaNivelRio::sinAgua(const QPixmap &p)
{
    QImage im = p.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < im.height(); ++y) {
        for (int x = 0; x < im.width(); ++x) {
            QRgb c = im.pixel(x, y);
            bool aguaPintada = qAlpha(c) > 0 && qAlpha(c) < 255;          // capa azul semitransparente
            bool brillo = c == qRgba(204, 229, 247, 255);                  // linea de la superficie
            if (aguaPintada || brillo)
                im.setPixel(x, y, qRgba(0, 0, 0, 0));
        }
    }
    return QPixmap::fromImage(im);
}

void EscenaNivelRio::reconstruir()
{
    etapaAnterior = static_cast<int>(nivel->getEtapa());
    estabaCapturado = false;
    crearApoyos();
    crearSpritesEntidades();
}

// ------------------------------------------------------------ Construccion

void EscenaNivelRio::crearApoyos()
{
    qDeleteAll(itemsApoyos);
    itemsApoyos.clear();
    for (const Apoyo &a : nivel->getApoyos()) {
        const QRectF &z = a.zona;
        if (a.tipo == TipoApoyo::ORILLA) {
            QGraphicsRectItem *tierra = addRect(QRectF(z.left(), z.top(), z.width(), NivelRio::ALTO - z.top()),
                                                Qt::NoPen, QColor(58, 48, 34));
            tierra->setZValue(Z_APOYOS);
            QGraphicsRectItem *pasto = addRect(QRectF(z.left(), z.top() - 4, z.width(), 6),
                                               Qt::NoPen, QColor(40, 70, 36));
            pasto->setZValue(Z_APOYOS + 1);
            itemsApoyos << tierra << pasto;
            continue;
        }
        const QPixmap &sp = (a.tipo == TipoApoyo::BANCO_ARENA) ? spBanco : spRoca;
        QGraphicsPixmapItem *item = addPixmap(sp);
        item->setTransform(QTransform::fromScale(z.width() / sp.width(), z.height() / sp.height()));
        item->setPos(z.topLeft());
        item->setZValue(Z_APOYOS);
        itemsApoyos << item;
    }
}

void EscenaNivelRio::crearSpritesEntidades()
{
    delete itemCarlos;
    qDeleteAll(itemsSoldados);
    qDeleteAll(itemsConos);
    qDeleteAll(barrasSospecha);
    qDeleteAll(signosSoldados);
    qDeleteAll(itemsAnimas);
    itemsSoldados.clear();
    itemsConos.clear();
    barrasSospecha.clear();
    signosSoldados.clear();
    itemsAnimas.clear();

    itemCarlos = addPixmap(spCarlosPie1);
    itemCarlos->setScale(ESCALA_CARLOS);
    itemCarlos->setZValue(Z_PERSONAJES + 1);

    for (int i = 0; i < nivel->getSoldados().size(); ++i) {
        QGraphicsPixmapItem *s = addPixmap(spSoldado);
        s->setOffset(-80, -80);
        s->setScale(0.5);
        s->setZValue(Z_PERSONAJES);
        itemsSoldados.append(s);

        QGraphicsPolygonItem *cono = addPolygon(QPolygonF(), Qt::NoPen, QColor(255, 240, 160, 70));
        cono->setZValue(Z_LUZ);
        itemsConos.append(cono);

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
    for (int i = 0; i < nivel->getAnimas().size(); ++i) {
        QGraphicsPixmapItem *a = addPixmap(spAnimaBlanca);
        a->setOffset(-32, -32);
        a->setZValue(Z_ANIMAS);
        itemsAnimas.append(a);
    }
}

// ------------------------------------------------------------ Bucle

void EscenaNivelRio::actualizarLogica(double dt)
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
        actualizarLatidos(dt);

        // Brazadas en la superficie: chapoteo; sobre las rocas: pasos
        esperaChapoteo -= dt;
        if (carlos->estaMoviendose() && esperaChapoteo <= 0.0) {
            if (carlos->estaEnAgua() && !carlos->estaSumergido()) {
                sonar("chapoteo", 0.45);
                esperaChapoteo = 0.6;
            } else if (!carlos->estaEnAgua()) {
                sonar("pasos", 0.25);
                esperaChapoteo = 0.45;
            }
        }
    }
    juego->actualizar(dt);

    int etapa = static_cast<int>(nivel->getEtapa());
    if (etapa != etapaAnterior && nivel->getEtapa() == EtapaRio::MEDIA
        && etapaAnterior == static_cast<int>(EtapaRio::NORMAL))
        sonar("creciente", 0.8);
    etapaAnterior = etapa;
    bool capturado = juego->getEstado() == EstadoJuego::CAPTURADO;
    if (capturado && !estabaCapturado)
        sonar("captura", 0.9);
    estabaCapturado = capturado;
}

void EscenaNivelRio::actualizarLatidos(double dt)
{
    Carlos *carlos = nivel->getCarlos();
    pulso = qMax(0.0, pulso - dt * 4.0);
    faseLatido += dt * carlos->ritmoLatidos() / 60.0;
    if (faseLatido >= 1.0) {
        faseLatido -= 1.0;
        pulso = 1.0;
#ifdef HAY_MULTIMEDIA
        double panico = carlos->getPanico();
        if (panico > 10.0) {
            sonidoLatido->setVolume(0.15 + 0.85 * panico / 100.0);
            sonidoLatido->play();
        }
#endif
    }
}

void EscenaNivelRio::teclaPulsada(int tecla)
{
    EstadoJuego estado = juego->getEstado();
    if (tecla == Qt::Key_P || tecla == Qt::Key_Escape) {
        juego->alternarPausa();
    } else if (tecla == Qt::Key_G && estado == EstadoJuego::PAUSADO) {
        emit guardarPartida();
    } else if (tecla == Qt::Key_M && (estado == EstadoJuego::PAUSADO || estado == EstadoJuego::CAPTURADO)) {
        emit volverAlMenu();
    } else if (tecla == Qt::Key_F1) {
        modoDepuracion = !modoDepuracion;
    } else if (tecla == Qt::Key_Return || tecla == Qt::Key_Enter) {
        if (estado == EstadoJuego::CAPTURADO) {
            juego->reiniciarNivel();
            crearSpritesEntidades();
        } else if (estado == EstadoJuego::FINAL || estado == EstadoJuego::NIVEL_SUPERADO) {
            emit siguienteNivel();
        }
    }
}

void EscenaNivelRio::actualizarVista()
{
    Carlos *carlos = nivel->getCarlos();
    if (carlos == nullptr)
        return;

    // --- Agua ---
    double sup = nivel->getSuperficie();
    agua->setRect(QRectF(0, sup, NivelRio::LARGO, NivelRio::ALTO - sup));
    QLinearGradient g(0, sup, 0, NivelRio::ALTO);
    g.setColorAt(0.0, QColor(36, 80, 112, 120));
    g.setColorAt(0.25, QColor(26, 58, 86, 190));
    g.setColorAt(1.0, QColor(8, 20, 34, 245));
    agua->setBrush(g);
    lineaAgua->setRect(QRectF(0, sup - 1, NivelRio::LARGO, 2));

    // --- Carlos ---
    QPointF p = carlos->getPosicion();
    const QPixmap *sp;
    double cintura;   // fila del sprite que va en la posicion de Carlos
    bool paso = static_cast<int>(tiempoAnimacion / 0.22) % 2 == 0;
    if (!carlos->estaEnAgua()) {
        sp = (carlos->estaMoviendose() && paso) ? &spCarlosPie2 : &spCarlosPie1;
        cintura = 130.0;   // los pies quedan sobre la roca
    } else if (carlos->estaSumergido()) {
        sp = &spCarlosSumergido;
        cintura = 80.0;
    } else if (carlos->getVelocidad().x() < -110.0) {
        sp = &spCarlosArrastrado;   // la corriente o un tronco se lo lleva
        cintura = 96.0;
    } else {
        sp = paso ? &spCarlosNada1 : &spCarlosNada2;
        cintura = 96.0;
    }
    if (itemCarlos->pixmap().cacheKey() != sp->cacheKey())
        itemCarlos->setPixmap(*sp);
    bool mirandoIzquierda = qAbs(carlos->getOrientacion()) > M_PI / 2.0;
    // Se voltea el sprite cuando nada hacia la izquierda
    QTransform t;
    t.scale(mirandoIzquierda ? -ESCALA_CARLOS : ESCALA_CARLOS, ESCALA_CARLOS);
    itemCarlos->setScale(1.0);
    itemCarlos->setTransform(t);
    itemCarlos->setOffset(-48, -cintura);
    itemCarlos->setPos(p);

    // --- Soldados y conos de luz ---
    const QList<Soldado *> &soldados = nivel->getSoldados();
    for (int i = 0; i < soldados.size() && i < itemsSoldados.size(); ++i) {
        const Soldado *s = soldados[i];
        bool izquierda = s->getVelocidad().x() < -0.1;
        QTransform ts;
        ts.scale(izquierda ? -0.5 : 0.5, 0.5);
        itemsSoldados[i]->setScale(1.0);
        itemsSoldados[i]->setTransform(ts);
        itemsSoldados[i]->setPos(s->getPosicion());

        const Linterna *l = s->getLinterna();
        double dir = l->direccionActual(), ap = l->getApertura(), alc = l->getAlcance();
        QPointF o = l->getOrigen();
        QPolygonF cono;
        cono << o;
        for (int k = 0; k <= 8; ++k) {
            double a = dir - ap + 2 * ap * k / 8.0;
            cono << o + QPointF(qCos(a), qSin(a)) * alc;
        }
        itemsConos[i]->setPolygon(cono);
        bool persigue = s->getEstado() == EstadoSoldado::PERSIGUIENDO;
        itemsConos[i]->setBrush(persigue ? QColor(255, 120, 90, 110) : QColor(255, 240, 160, 70));

        QGraphicsSimpleTextItem *signo = signosSoldados[i];
        signo->setVisible(s->getEstado() == EstadoSoldado::INVESTIGANDO);
        signo->setPos(s->getPosicion() + QPointF(-6, -64));
        QGraphicsRectItem *barra = barrasSospecha[i];
        if (s->getSospecha() > 0.01 && !persigue) {
            barra->setRect(0, 0, 40 * s->getSospecha(), 5);
            barra->setPos(s->getPosicion() + QPointF(-20, -46));
            barra->show();
        } else {
            barra->hide();
        }
    }

    actualizarTroncos();
    actualizarAnimas();

    // --- Camara: sigue a Carlos en x (la escena mide 600 de alto, como la ventana) ---
    if (!views().isEmpty())
        views().first()->centerOn(QPointF(p.x() + 120.0, NivelRio::ALTO / 2.0));
}

void EscenaNivelRio::actualizarTroncos()
{
    const QList<Tronco *> &troncos = nivel->getTroncos();
    for (int i = 0; i < itemsTroncos.size(); ++i) {
        QGraphicsPixmapItem *item = itemsTroncos[i];
        if (i >= troncos.size()) {
            item->hide();
            continue;
        }
        const Tronco *t = troncos[i];
        const QPixmap &sp = (t->getTipo() == TipoTronco::TRONCO) ? spTronco : spEscombros;
        if (item->pixmap().cacheKey() != sp.cacheKey())
            item->setPixmap(sp);
        double escala = t->getTamano().width() / sp.width();
        item->setOffset(-sp.width() / 2.0, -sp.height() / 2.0);
        item->setScale(escala);
        item->setRotation(qRadiansToDegrees(t->getInclinacion()));
        item->setPos(t->getPosicion());
        item->show();
    }
}

void EscenaNivelRio::actualizarAnimas()
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

// ------------------------------------------------------------ Dibujo

// Cielo y monte del fondo con paralaje (se mueve mas lento que la camara)
void EscenaNivelRio::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(10, 14, 30));
    int etapa = static_cast<int>(nivel->getEtapa());
    const QPixmap &fondo = spFondo[etapa];
    if (fondo.isNull())
        return;
    // Se usa la franja central de la imagen (sin las orillas pintadas) hasta el agua
    QRectF origen(0, 0, 480, 170);   // sin la luna (se repetiria)
    double escala = 600.0 / 360.0;
    double ancho = origen.width() * escala, alto = origen.height() * escala;
    double camara = rect.left();
    double desplazamiento = std::fmod(camara * 0.6, ancho);
    for (double x = camara - desplazamiento; x < rect.right(); x += ancho)
        painter->drawPixmap(QRectF(x, 0, ancho, alto), fondo, origen);
    // Debajo del monte, oscuro hasta el agua
    painter->fillRect(QRectF(rect.left(), alto, rect.width(), 600 - alto), QColor(14, 28, 20));
    // Barranco de la orilla de enfrente, donde caminan los soldados
    painter->fillRect(QRectF(rect.left(), NivelRio::Y_ORILLA_SOLDADOS + 26, rect.width(), 8),
                      QColor(40, 34, 24));
}

void EscenaNivelRio::dibujarDepuracion(QPainter *painter)
{
    const char *nombres[] = {"GUIAR", "ADVERTIR", "OCULTARSE", "CALMAR"};
    for (const Anima *a : nivel->getAnimas()) {
        const Decision &d = a->getDecision();
        painter->setPen(QPen(QColor(200, 220, 255, 200), 1, Qt::DashLine));
        painter->drawLine(a->getPosicion(), d.destino);
        painter->setPen(QColor(230, 240, 255));
        painter->drawText(a->getPosicion() + QPointF(14, -10), nombres[static_cast<int>(d.tipo)]);
    }
    // Apoyos: verde si estan secos, rojo si los tapo el agua
    for (const Apoyo &a : nivel->getApoyos()) {
        painter->setPen(QPen(nivel->apoyoSeco(a) ? QColor(90, 220, 90) : QColor(230, 70, 60), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(a.zona.adjusted(0, 0, 0, -a.zona.height() + 6));
    }
}

void EscenaNivelRio::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (views().isEmpty())
        return;
    QWidget *pantalla = views().first()->viewport();
    const int w = pantalla->width(), h = pantalla->height();

    // Borde de la corriente: si Carlos queda detras de esta linea, se lo lleva el rio
    double limite = nivel->getLimiteArrastre();
    if (limite > rect.left() - 120) {
        QLinearGradient g(limite - 120, 0, limite + 60, 0);
        g.setColorAt(0.0, QColor(10, 20, 40, 230));
        g.setColorAt(1.0, QColor(10, 20, 40, 0));
        painter->fillRect(QRectF(limite - 120, 0, 180, h), g);
    }
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
    painter->drawText(12, 22, "Nivel 2: El rio   |   cruza hasta la otra orilla (derecha)");
    painter->drawText(12, h - 12, "A/D nadar  S bucear  W subir a una roca  P pausa  F1 animas");

    Carlos *carlos = nivel->getCarlos();
    if (carlos != nullptr) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        // Corazon y panico
        double panico = carlos->getPanico();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(200 + 55 * pulso, 40, 50));
        double radio = 9 + 4 * pulso;
        painter->drawEllipse(QPointF(w - 230, 20), radio, radio);
        painter->setBrush(QColor(255, 255, 255, 40));
        painter->drawRect(w - 212, 14, 150, 12);
        painter->setBrush(panico > 60 ? QColor(220, 60, 50) : QColor(220, 170, 60));
        painter->drawRect(QRectF(w - 212, 14, 1.5 * panico, 12));

        // Aire
        painter->setPen(QColor(200, 225, 240));
        painter->drawText(w - 262, 49, "Aire");
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 40));
        painter->drawRect(w - 212, 38, 150, 12);
        double aire = carlos->getAire();
        painter->setBrush(aire < 30 ? QColor(230, 90, 70) : QColor(120, 190, 235));
        painter->drawRect(QRectF(w - 212, 38, 1.5 * aire, 12));

        // Indicador de la creciente
        double falta = nivel->tiempoParaCreciente();
        QString texto;
        QColor color(200, 225, 240);
        switch (nivel->getEtapa()) {
        case EtapaRio::NORMAL:
            texto = QString("Rio tranquilo - creciente en %1 s").arg(qCeil(falta));
            if (falta < 6.0)
                color = QColor(240, 200, 90);
            break;
        case EtapaRio::MEDIA:
            texto = "El rio esta creciendo";
            color = QColor(240, 170, 80);
            break;
        case EtapaRio::CRECIENTE:
            texto = "¡CRECIENTE! Agarrate de una roca alta";
            color = QColor(255, 110, 90);
            break;
        }
        painter->setPen(color);
        painter->drawText(12, 44, texto);
        painter->setPen(QColor(200, 225, 240));
        painter->drawText(12, 64, QString("Corriente: %1 px/s").arg(qAbs(nivel->getVelocidadRio()), 0, 'f', 0));
        if (!carlos->estaEnAgua())
            painter->drawText(QRect(0, h - 70, w, 20), Qt::AlignHCenter, "Sobre la roca: respiras, pero te pueden ver");
        else if (carlos->estaSumergido())
            painter->drawText(QRect(0, h - 70, w, 20), Qt::AlignHCenter, "Bajo el agua: las linternas no te ven");

        if (panico > 60) {
            QRadialGradient v(QPointF(w / 2.0, h / 2.0), w * 0.7);
            v.setColorAt(0.6, QColor(60, 0, 0, 0));
            v.setColorAt(1.0, QColor(60, 0, 0, int((panico - 60) * 3 * (0.6 + 0.4 * pulso))));
            painter->fillRect(0, 0, w, h, v);
        }
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    QString mensaje;
    switch (juego->getEstado()) {
    case EstadoJuego::CAPTURADO:
        switch (nivel->getMotivo()) {
        case MotivoDerrota::AHOGADO:    mensaje = "Te quedaste sin aire."; break;
        case MotivoDerrota::ARRASTRADO: mensaje = "La corriente te arrastro."; break;
        default:                        mensaje = "Te vieron desde la orilla."; break;
        }
        mensaje += "\n\nEnter: intentarlo de nuevo     M: menu principal";
        break;
    case EstadoJuego::PAUSADO:
        mensaje = "Pausa\n\nP: continuar     G: guardar partida     M: menu principal";
        break;
    case EstadoJuego::FINAL:
    case EstadoJuego::NIVEL_SUPERADO:
        mensaje = "Cruzaste el rio.\n\nCucuta esta cerca.\nPresiona Enter";
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
