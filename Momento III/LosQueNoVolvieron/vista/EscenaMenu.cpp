#include "EscenaMenu.h"
#include <QPainter>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QtMath>

EscenaMenu::EscenaMenu(QObject *parent)
    : EscenaJuego(parent), seleccion(NUEVA), hayPartida(false), viendoControles(false), tiempo(0.0)
{
    setSceneRect(0, 0, 960, 600);
    fondo = QPixmap(":/sprites/nivel2_rio/fondo_rio_normal.png");
    spAnima = QPixmap(":/sprites/animas/anima_blanca_segura.png");
    spRamon = QPixmap(":/sprites/animas/anima_ramon.png");
}

void EscenaMenu::setHayPartida(bool hay)
{
    hayPartida = hay;
    seleccion = hay ? CONTINUAR : NUEVA;
    viendoControles = false;
}

void EscenaMenu::actualizarLogica(double dt)
{
    tiempo += dt;
}

void EscenaMenu::actualizarVista()
{
    update();
}

void EscenaMenu::mover(int paso)
{
    do {
        seleccion = (seleccion + paso + NUM_OPCIONES) % NUM_OPCIONES;
    } while (seleccion == CONTINUAR && !hayPartida);   // Continuar se salta si no hay partida
    sonar("pasos", 0.3);
}

void EscenaMenu::teclaPulsada(int tecla)
{
    if (viendoControles) {
        viendoControles = false;   // cualquier tecla vuelve al menu
        return;
    }
    switch (tecla) {
    case Qt::Key_W: case Qt::Key_Up:
        mover(-1);
        break;
    case Qt::Key_S: case Qt::Key_Down:
        mover(1);
        break;
    case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_Space:
        switch (seleccion) {
        case NUEVA:     emit nuevaPartida(); break;
        case CONTINUAR: emit continuarPartida(); break;
        case CONTROLES: viendoControles = true; break;
        case SALIR:     emit salir(); break;
        }
        break;
    case Qt::Key_Escape:
        emit salir();
        break;
    default:
        break;
    }
}

void EscenaMenu::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(8, 10, 18));
    if (!fondo.isNull()) {
        // El rio de noche, ocupando toda la pantalla (se recorta a lo ancho)
        painter->drawPixmap(QRectF(0, 0, 960, 600), fondo, QRectF(0, 0, 576, 360));
    }
    QLinearGradient g(0, 0, 0, 600);
    g.setColorAt(0.0, QColor(0, 0, 0, 120));
    g.setColorAt(0.6, QColor(0, 0, 0, 160));
    g.setColorAt(1.0, QColor(0, 0, 0, 230));
    painter->fillRect(QRectF(0, 0, 960, 600), g);

    // Animas que flotan sobre el agua (vaiven armonico, como en el juego)
    const QPointF base[4] = {QPointF(150, 360), QPointF(300, 410), QPointF(700, 380), QPointF(830, 330)};
    for (int i = 0; i < 4; ++i) {
        const QPixmap &p = (i == 3) ? spRamon : spAnima;
        if (p.isNull())
            continue;
        double y = base[i].y() + 10.0 * qSin(1.7 * tiempo + i * 1.3);
        painter->setOpacity(0.55 + 0.35 * qSin(0.9 * tiempo + i));
        painter->drawPixmap(QPointF(base[i].x() - p.width() / 2.0, y - p.height() / 2.0), p);
    }
    painter->setOpacity(1.0);
}

void EscenaMenu::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QFont f = painter->font();

    f.setPointSize(38);
    f.setBold(true);
    painter->setFont(f);
    painter->setPen(QColor(0, 0, 0, 200));
    painter->drawText(QRect(3, 63, 960, 70), Qt::AlignCenter, "Los que no volvieron");
    painter->setPen(QColor(232, 226, 205));
    painter->drawText(QRect(0, 60, 960, 70), Qt::AlignCenter, "Los que no volvieron");

    f.setPointSize(13);
    f.setBold(false);
    f.setItalic(true);
    painter->setFont(f);
    painter->setPen(QColor(190, 190, 175));
    painter->drawText(QRect(0, 130, 960, 30), Qt::AlignCenter, "Catatumbo, 2002 - 2008");
    f.setItalic(false);

    if (viendoControles) {
        painter->fillRect(QRect(150, 190, 660, 330), QColor(0, 0, 0, 200));
        f.setPointSize(12);
        painter->setFont(f);
        painter->setPen(QColor(225, 222, 205));
        painter->drawText(QRect(180, 205, 600, 300), Qt::AlignLeft | Qt::TextWordWrap,
            "NIVEL 1 - LA SELVA\n"
            "WASD o flechas: moverse     Shift: correr\n"
            "Espacio (mantener y soltar): lanzar una piedra para distraer\n"
            "E: esconderse en matorrales o troncos huecos\n\n"
            "NIVEL 2 - EL RÍO\n"
            "A/D: nadar     S: bucear (gasta aire)     W: subirse a una roca\n\n"
            "P: pausa (G guarda la partida, M vuelve al menú)\n"
            "F1: ver lo que piensan las ánimas     Enter: reintentar\n\n"
            "Las luces que flotan son las ánimas. Blancas: por ahí es seguro.\n"
            "Rojas: peligro. Síguelas.");
        f.setPointSize(10);
        painter->setFont(f);
        painter->setPen(QColor(160, 160, 150));
        painter->drawText(QRect(0, 530, 960, 20), Qt::AlignCenter, "Presiona cualquier tecla para volver");
        painter->restore();
        return;
    }

    const char *nombres[NUM_OPCIONES] = {"Nueva partida", "Continuar", "Controles", "Salir"};
    f.setPointSize(17);
    for (int i = 0; i < NUM_OPCIONES; ++i) {
        bool activa = (i != CONTINUAR || hayPartida);
        bool elegida = (i == seleccion);
        f.setBold(elegida);
        painter->setFont(f);
        QColor c = !activa ? QColor(95, 95, 90) : (elegida ? QColor(250, 235, 170) : QColor(205, 205, 195));
        painter->setPen(c);
        QRect caja(0, 240 + i * 52, 960, 40);
        QString texto = nombres[i];
        if (elegida)
            texto = QString::fromUtf8("·  ") + texto + QString::fromUtf8("  ·");
        painter->drawText(caja, Qt::AlignCenter, texto);
    }

    f.setPointSize(11);
    f.setBold(false);
    painter->setFont(f);
    painter->setPen(QColor(170, 170, 160));
    painter->drawText(QRect(80, 500, 800, 60), Qt::AlignCenter | Qt::TextWordWrap,
        "Carlos vio lo que no debía ver. Tiene que llegar a Cúcuta para contarlo.\n"
        "No está solo: los que no volvieron le muestran el camino.");
    dibujarAviso(painter, 960, 600);
    painter->restore();
}
