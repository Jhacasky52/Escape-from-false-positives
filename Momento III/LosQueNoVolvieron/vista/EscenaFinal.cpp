#include "EscenaFinal.h"
#include <QPainter>
#include <QImage>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>

namespace {
const double SUELO = 470.0;    // donde camina Carlos
const double X_PARADA = 430.0; // donde se detiene

// Quita el agua semitransparente que traen pintada los sprites del rio
QPixmap sinAgua(const QString &ruta)
{
    QImage im = QImage(ruta).convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < im.height(); ++y)
        for (int x = 0; x < im.width(); ++x) {
            QRgb c = im.pixel(x, y);
            if ((qAlpha(c) > 0 && qAlpha(c) < 255) || c == qRgba(204, 229, 247, 255))
                im.setPixel(x, y, qRgba(0, 0, 0, 0));
        }
    return QPixmap::fromImage(im);
}

// 0 antes de 'desde', 1 despues de 'hasta', lineal en medio
double rampa(double t, double desde, double hasta)
{
    return qBound(0.0, (t - desde) / (hasta - desde), 1.0);
}
}

EscenaFinal::EscenaFinal(QObject *parent)
    : EscenaJuego(parent), tiempo(0.0), sonoRamon(false)
{
    setSceneRect(0, 0, 960, 600);
    spCarlos1 = sinAgua(":/sprites/nivel2_rio/carlos_lado_walk1.png");
    spCarlos2 = sinAgua(":/sprites/nivel2_rio/carlos_lado_walk2.png");
    spRamon = QPixmap(":/sprites/animas/anima_ramon.png");
    spAnima = QPixmap(":/sprites/animas/anima_blanca_segura.png");

    // Silueta de Cucuta: edificios de alturas variadas (fijos, sin azar)
    double x = 0.0;
    int i = 0;
    while (x < 960.0) {
        double ancho = 38.0 + (i * 37 % 50);
        double alto = 50.0 + (i * 53 % 120) + (i % 4 == 0 ? 60.0 : 0.0);
        edificios.append(QRectF(x, SUELO - 40 - alto, ancho, alto));
        for (int v = 0; v < 3; ++v)
            if ((i + v) % 3 == 0)
                ventanas.append(QPointF(x + 8 + v * 11, SUELO - 40 - alto + 14 + (i * 7 + v * 13) % int(alto - 20)));
        x += ancho + 4.0;
        ++i;
    }
}

void EscenaFinal::comenzar()
{
    tiempo = 0.0;
    sonoRamon = false;
}

void EscenaFinal::actualizarLogica(double dt)
{
    tiempo += dt;
    if (!sonoRamon && tiempo > 5.0) {
        sonar("ramon", 0.7);
        sonoRamon = true;
    }
}

void EscenaFinal::actualizarVista()
{
    update();
}

void EscenaFinal::teclaPulsada(int tecla)
{
    if (tecla != Qt::Key_Return && tecla != Qt::Key_Enter && tecla != Qt::Key_Space && tecla != Qt::Key_Escape)
        return;
    if (tiempo < DURACION)
        tiempo = DURACION;     // adelantar al texto final
    else
        emit volverAlMenu();
}

QPointF EscenaFinal::posicionCarlos() const
{
    // Camina de la izquierda hasta X_PARADA en 7 s
    double x = -40.0 + (X_PARADA + 40.0) * rampa(tiempo, 0.0, 7.0);
    return QPointF(x, SUELO);
}

QPointF EscenaFinal::posicionRamon() const
{
    // Baja desde arriba a la derecha y se queda frente a Carlos; al final sube al cielo
    QPointF inicio(820, 90), frente(X_PARADA + 70, SUELO - 70), cielo(X_PARADA + 120, -60);
    double llegar = rampa(tiempo, 4.0, 9.0);
    double subir = rampa(tiempo, 15.0, 21.0);
    QPointF p = inicio + (frente - inicio) * (1 - qCos(llegar * M_PI)) / 2.0;
    p += (cielo - frente) * subir * subir;
    p.ry() += 8.0 * qSin(tiempo * 1.8);   // vaiven armonico
    return p;
}

void EscenaFinal::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    // Amanecer: de noche azul a naranja en el horizonte
    double luz = rampa(tiempo, 0.0, 14.0);
    QLinearGradient cielo(0, 0, 0, SUELO);
    cielo.setColorAt(0.0, QColor(int(12 + 40 * luz), int(16 + 50 * luz), int(40 + 70 * luz)));
    cielo.setColorAt(0.75, QColor(int(30 + 150 * luz), int(30 + 90 * luz), int(60 + 40 * luz)));
    cielo.setColorAt(1.0, QColor(int(60 + 180 * luz), int(40 + 120 * luz), int(50 + 30 * luz)));
    painter->fillRect(QRectF(0, 0, 960, 600), cielo);

    // Sol que asoma detras de la ciudad
    QRadialGradient sol(QPointF(700, SUELO - 40 + 90 * (1 - luz)), 220);
    sol.setColorAt(0.0, QColor(255, 220, 150, int(220 * luz)));
    sol.setColorAt(0.25, QColor(255, 170, 90, int(120 * luz)));
    sol.setColorAt(1.0, QColor(255, 150, 80, 0));
    painter->fillRect(QRectF(0, 0, 960, 600), sol);

    // Ciudad y luces que se apagan al llegar el dia
    QColor silueta(int(18 + 30 * luz), int(16 + 22 * luz), int(26 + 20 * luz));
    for (const QRectF &e : edificios)
        painter->fillRect(e, silueta);
    painter->fillRect(QRectF(0, SUELO - 40, 960, 40), silueta.darker(110));
    for (const QPointF &v : ventanas)
        painter->fillRect(QRectF(v, QSizeF(5, 6)), QColor(250, 210, 120, int(200 * (1 - luz))));
    // Carretera
    painter->fillRect(QRectF(0, SUELO, 960, 600 - SUELO), QColor(int(28 + 30 * luz), int(24 + 24 * luz), int(22 + 16 * luz)));
    painter->fillRect(QRectF(0, SUELO + 2, 960, 3), QColor(80, 70, 55));
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(200, 190, 140, 90));
    for (int x = 0; x < 960; x += 80)
        painter->drawRect(QRectF(x + 20, SUELO + 60, 40, 4));
    // Letrero
    painter->fillRect(QRectF(760, SUELO - 70, 6, 70), QColor(70, 70, 70));
    painter->fillRect(QRectF(712, SUELO - 98, 104, 34), QColor(30, 90, 50));
    painter->setPen(QColor(235, 235, 225));
    QFont f = painter->font();
    f.setPointSize(11);
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(QRectF(712, SUELO - 98, 104, 34), Qt::AlignCenter, QString::fromUtf8("CÚCUTA"));
}

void EscenaFinal::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

    // Carlos caminando (dos cuadros) o quieto
    QPointF c = posicionCarlos();
    bool camina = tiempo < 7.0;
    const QPixmap &sp = (camina && int(tiempo / 0.25) % 2 == 0) ? spCarlos2 : spCarlos1;
    painter->drawPixmap(QRectF(c.x() - 24, c.y() - 79, 48, 80), sp, sp.rect());

    // Las otras animas aparecen junto a Ramon y suben con el
    double otras = rampa(tiempo, 11.0, 13.0) * (1 - rampa(tiempo, 19.0, 21.0));
    for (int i = 0; i < 3 && !spAnima.isNull(); ++i) {
        QPointF base(X_PARADA - 120 + i * 60, SUELO - 120 - i * 25);
        base.ry() -= 500 * qPow(rampa(tiempo, 15.0 + i * 0.6, 21.0), 2);
        base.ry() += 7 * qSin(tiempo * 1.6 + i);
        painter->setOpacity(0.8 * otras);
        painter->drawPixmap(base - QPointF(spAnima.width() / 2.0, spAnima.height() / 2.0), spAnima);
    }

    // Ramon: su luz brilla mas en el reencuentro
    if (!spRamon.isNull()) {
        double visible = rampa(tiempo, 3.0, 5.0) * (1 - rampa(tiempo, 19.0, 21.0));
        QPointF r = posicionRamon();
        QRadialGradient halo(r, 90);
        halo.setColorAt(0.0, QColor(255, 200, 120, int(120 * visible)));
        halo.setColorAt(1.0, QColor(255, 200, 120, 0));
        painter->setOpacity(1.0);
        painter->fillRect(QRectF(r.x() - 90, r.y() - 90, 180, 180), halo);
        painter->setOpacity(visible);
        double escala = 1.0 + 0.15 * qSin(tiempo * 3.0);
        QSizeF tam(spRamon.width() * escala, spRamon.height() * escala);
        painter->drawPixmap(QRectF(r - QPointF(tam.width() / 2, tam.height() / 2), tam), spRamon, spRamon.rect());
    }
    painter->setOpacity(1.0);

    // Textos de la historia
    struct Texto { double desde, hasta; const char *texto; };
    const Texto textos[] = {
        {1.0, 6.0, "Carlos llegó a Cúcuta al amanecer."},
        {7.0, 12.0, "Ramón estaba ahí. Lo había acompañado todo el camino."},
        {12.5, 18.0, "Esa mañana Carlos contó lo que vio."},
        {18.5, 23.0, "Ellos ya pueden descansar."},
    };
    QFont f = painter->font();
    f.setPointSize(16);
    f.setItalic(true);
    painter->setFont(f);
    for (const Texto &t : textos) {
        double a = rampa(tiempo, t.desde, t.desde + 0.8) * (1 - rampa(tiempo, t.hasta - 0.8, t.hasta));
        if (a <= 0.0)
            continue;
        painter->setPen(QColor(0, 0, 0, int(180 * a)));
        painter->drawText(QRect(2, 82, 960, 40), Qt::AlignCenter, QString::fromUtf8(t.texto));
        painter->setPen(QColor(245, 238, 220, int(255 * a)));
        painter->drawText(QRect(0, 80, 960, 40), Qt::AlignCenter, QString::fromUtf8(t.texto));
    }

    // Cierre: pantalla negra con la dedicatoria
    double cierre = rampa(tiempo, DURACION - 2.0, DURACION);
    if (cierre > 0.0) {
        painter->fillRect(QRectF(0, 0, 960, 600), QColor(0, 0, 0, int(255 * cierre)));
        f.setItalic(false);
        f.setBold(true);
        f.setPointSize(30);
        painter->setFont(f);
        painter->setPen(QColor(235, 230, 215, int(255 * cierre)));
        painter->drawText(QRect(0, 150, 960, 60), Qt::AlignCenter, "Los que no volvieron");
        f.setBold(false);
        f.setPointSize(13);
        painter->setFont(f);
        painter->drawText(QRect(120, 240, 720, 140), Qt::AlignCenter | Qt::TextWordWrap, QString::fromUtf8(
            "En memoria de las víctimas de las ejecuciones extrajudiciales\n"
            "conocidas como \"falsos positivos\".\n\n"
            "La Jurisdicción Especial para la Paz (JEP) ha documentado\n"
            "al menos 6.402 víctimas entre 2002 y 2008."));
        f.setPointSize(11);
        painter->setFont(f);
        painter->setPen(QColor(170, 170, 160, int(255 * cierre)));
        painter->drawText(QRect(0, 520, 960, 30), Qt::AlignCenter, QString::fromUtf8("Presiona Enter para volver al menú"));
    }
    painter->restore();
}
