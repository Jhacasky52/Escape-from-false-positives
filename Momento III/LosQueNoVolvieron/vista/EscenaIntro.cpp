#include "EscenaIntro.h"
#include <QPainter>
#include <QImage>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>

namespace {
const double SUELO = 440.0;

// Bloques de la historia: desde que segundo se muestra cada texto
struct Bloque { double desde; const char *texto; };
const Bloque BLOQUES[] = {
    {0.0,  "Catatumbo, Norte de Santander. 2007."},
    {6.0,  "Carlos y Ramón crecieron juntos en la vereda.\nSembraban, pescaban y soñaban con irse algún día al pueblo."},
    {13.0, "Una noche llegaron unos hombres ofreciéndole a Ramón\ntrabajo lejos de la vereda. Buena paga, dijeron."},
    {20.0, "Ramón se fue con ellos. No volvió."},
    {26.0, "Días después lo presentaron como un guerrillero muerto en combate.\nCarlos sabía que era mentira: vio cuando se lo llevaron."},
    {33.0, "Ahora lo buscan a él."},
    {38.0, "Esa noche, en el monte, una luz tibia se le acercó.\nCarlos supo que era Ramón."},
};
const int NUM_BLOQUES = sizeof(BLOQUES) / sizeof(BLOQUES[0]);
const double FIN = 45.0;

double rampa(double t, double desde, double hasta)
{
    return qBound(0.0, (t - desde) / (hasta - desde), 1.0);
}

// Quita el agua pintada en los sprites del rio; con 'camisa' se pinta a Ramon
QPixmap preparar(const QString &ruta, bool esRamon)
{
    QImage im = QImage(ruta).convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < im.height(); ++y)
        for (int x = 0; x < im.width(); ++x) {
            QRgb c = im.pixel(x, y);
            if ((qAlpha(c) > 0 && qAlpha(c) < 255) || c == qRgba(204, 229, 247, 255))
                im.setPixel(x, y, qRgba(0, 0, 0, 0));
            else if (esRamon && c == qRgba(226, 220, 200, 255))
                im.setPixel(x, y, qRgba(150, 58, 48, 255));     // camisa roja
            else if (esRamon && c == qRgba(216, 190, 112, 255))
                im.setPixel(x, y, qRgba(70, 62, 52, 255));      // gorra oscura
        }
    return QPixmap::fromImage(im);
}
}

EscenaIntro::EscenaIntro(QObject *parent)
    : EscenaJuego(parent), tiempo(0.0), sonoRamon(false)
{
    setSceneRect(0, 0, 960, 600);
    const QString r = ":/sprites/nivel2_rio/";
    spCarlos1 = preparar(r + "carlos_lado_walk1.png", false);
    spCarlos2 = preparar(r + "carlos_lado_walk2.png", false);
    spRamon1 = preparar(r + "carlos_lado_walk1.png", true);
    spRamon2 = preparar(r + "carlos_lado_walk2.png", true);
    spAnimaRamon = QPixmap(":/sprites/animas/anima_ramon.png");
    for (int i = 0; i < 14; ++i)
        luciernagas.append(QPointF(60 + (i * 137) % 880, 250 + (i * 71) % 170));
}

void EscenaIntro::comenzar()
{
    tiempo = 0.0;
    sonoRamon = false;
}

void EscenaIntro::actualizarLogica(double dt)
{
    tiempo = qMin(FIN, tiempo + dt);
    if (!sonoRamon && tiempo > 39.0) {
        sonar("ramon", 0.6);
        sonoRamon = true;
    }
}

void EscenaIntro::actualizarVista()
{
    update();
}

void EscenaIntro::teclaPulsada(int tecla)
{
    if (tecla == Qt::Key_Escape) {
        emit terminada();   // saltar la introduccion
        return;
    }
    if (tecla != Qt::Key_Return && tecla != Qt::Key_Enter && tecla != Qt::Key_Space)
        return;
    if (tiempo >= FIN - 0.5) {
        emit terminada();
        return;
    }
    // Pasa al siguiente bloque de texto
    for (int i = 0; i < NUM_BLOQUES; ++i)
        if (BLOQUES[i].desde > tiempo + 0.1) {
            tiempo = BLOQUES[i].desde;
            return;
        }
    tiempo = FIN;
}

void EscenaIntro::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    // Despues de que Ramon se va, la noche se vuelve mas oscura
    double sombra = rampa(tiempo, 20.0, 26.0);
    QLinearGradient cielo(0, 0, 0, SUELO);
    cielo.setColorAt(0.0, QColor(10, 14, int(34 - 14 * sombra)));
    cielo.setColorAt(1.0, QColor(int(28 - 12 * sombra), int(34 - 14 * sombra), int(50 - 20 * sombra)));
    painter->fillRect(QRectF(0, 0, 960, 600), cielo);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(200, 200, 210, 140));
    for (int i = 0; i < 40; ++i)
        painter->drawRect(QRectF((i * 211) % 960, (i * 97) % 260, 2, 2));

    // Montanas del Catatumbo
    QPolygonF montes;
    montes << QPointF(0, SUELO - 120);
    for (int x = 0; x <= 960; x += 40)
        montes << QPointF(x, SUELO - 150 + 40 * qSin(x * 0.011) + 25 * qSin(x * 0.031 + 1.0));
    montes << QPointF(960, SUELO) << QPointF(0, SUELO);
    painter->setBrush(QColor(14, 26, 20));
    painter->drawPolygon(montes);
    painter->fillRect(QRectF(0, SUELO, 960, 600 - SUELO), QColor(24, 30, 18));

    // El rancho con la ventana encendida
    painter->fillRect(QRectF(120, SUELO - 90, 150, 90), QColor(52, 40, 28));
    QPolygonF techo;
    techo << QPointF(105, SUELO - 88) << QPointF(195, SUELO - 140) << QPointF(285, SUELO - 88);
    painter->setBrush(QColor(70, 58, 36));
    painter->drawPolygon(techo);
    QRadialGradient vela(QPointF(220, SUELO - 55), 70);
    vela.setColorAt(0.0, QColor(255, 190, 90, int(150 * (1 - 0.6 * sombra))));
    vela.setColorAt(1.0, QColor(255, 190, 90, 0));
    painter->fillRect(QRectF(150, SUELO - 125, 140, 140), vela);
    painter->fillRect(QRectF(205, SUELO - 68, 26, 22), QColor(240, 180, 90, int(230 * (1 - 0.5 * sombra))));

    // Carro con luces que llega por la derecha (13 s) y se va (22 s)
    double llega = rampa(tiempo, 13.0, 16.0), seVa = rampa(tiempo, 21.0, 24.0);
    double xCarro = 1000 - 300 * (1 - qCos(llega * M_PI)) / 2 + 500 * seVa * seVa;
    if (llega > 0.0 && xCarro < 1100) {
        // Haz de luz de los faros: un trapecio que se desvanece hacia la izquierda
        QLinearGradient faro(xCarro, 0, xCarro - 380, 0);
        faro.setColorAt(0.0, QColor(255, 240, 180, 140));
        faro.setColorAt(1.0, QColor(255, 240, 180, 0));
        QPolygonF haz;
        haz << QPointF(xCarro, SUELO - 40) << QPointF(xCarro - 380, SUELO - 120)
            << QPointF(xCarro - 380, SUELO + 20) << QPointF(xCarro, SUELO - 22);
        painter->setBrush(faro);
        painter->drawPolygon(haz);
        painter->fillRect(QRectF(xCarro, SUELO - 50, 150, 38), QColor(20, 22, 20));
        painter->fillRect(QRectF(xCarro + 40, SUELO - 78, 80, 30), QColor(20, 22, 20));
        painter->setBrush(QColor(10, 10, 10));
        painter->drawEllipse(QPointF(xCarro + 30, SUELO - 10), 12, 12);
        painter->drawEllipse(QPointF(xCarro + 120, SUELO - 10), 12, 12);
    }

    // Linternas de los soldados que buscan a Carlos (desde los 33 s)
    double busqueda = rampa(tiempo, 33.0, 35.0);
    if (busqueda > 0.0) {
        for (int i = 0; i < 2; ++i) {
            QPointF o(i == 0 ? -20.0 : 980.0, SUELO - 60 - 30 * i);
            double base = i == 0 ? 0.0 : M_PI;
            double a = base + (i == 0 ? 1 : -1) * (0.15 + 0.25 * qSin(tiempo * 0.9 + i));
            QPolygonF cono;
            cono << o << o + QPointF(qCos(a - 0.2), qSin(a - 0.2)) * 520 << o + QPointF(qCos(a + 0.2), qSin(a + 0.2)) * 520;
            painter->setBrush(QColor(255, 240, 160, int(55 * busqueda)));
            painter->drawPolygon(cono);
        }
    }
}

void EscenaIntro::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->save();

    // Luciernagas
    painter->setPen(Qt::NoPen);
    for (int i = 0; i < luciernagas.size(); ++i) {
        double brillo = 0.5 + 0.5 * qSin(tiempo * 2.0 + i * 1.7);
        QPointF p = luciernagas[i] + QPointF(10 * qSin(tiempo * 0.5 + i), 6 * qCos(tiempo * 0.7 + i));
        painter->setBrush(QColor(220, 240, 120, int(160 * brillo * (1 - rampa(tiempo, 31, 33)))));
        painter->drawEllipse(p, 2, 2);
    }

    // Carlos: quieto junto al rancho; al final sale corriendo hacia la derecha
    double huye = rampa(tiempo, 41.0, 45.0);
    QPointF carlos(330 + 120 * huye, SUELO);
    const QPixmap &sc = (huye > 0 && int(tiempo / 0.2) % 2) ? spCarlos2 : spCarlos1;
    painter->drawPixmap(QRectF(carlos.x() - 24, carlos.y() - 79, 48, 80), sc, sc.rect());

    // Ramon: a su lado, luego camina hacia el carro y desaparece
    double camina = rampa(tiempo, 16.0, 20.0);
    double ramonVisible = 1.0 - rampa(tiempo, 19.0, 20.5);
    if (ramonVisible > 0.0) {
        QPointF ramon(380 + 420 * camina, SUELO);
        const QPixmap &sr = (camina > 0 && camina < 1 && int(tiempo / 0.25) % 2) ? spRamon2 : spRamon1;
        painter->setOpacity(ramonVisible);
        // Mira hacia Carlos mientras hablan (volteado)
        if (camina <= 0.0) {
            painter->save();
            painter->translate(ramon.x(), 0);
            painter->scale(-1, 1);
            painter->drawPixmap(QRectF(-24, ramon.y() - 79, 48, 80), sr, sr.rect());
            painter->restore();
        } else
            painter->drawPixmap(QRectF(ramon.x() - 24, ramon.y() - 79, 48, 80), sr, sr.rect());
        painter->setOpacity(1.0);
    }

    // El anima de Ramon aparece junto a Carlos
    double anima = rampa(tiempo, 38.5, 40.5);
    if (anima > 0.0 && !spAnimaRamon.isNull()) {
        QPointF a(carlos.x() + 55, SUELO - 95 + 8 * qSin(tiempo * 1.8));
        QRadialGradient halo(a, 110);
        halo.setColorAt(0.0, QColor(255, 200, 120, int(130 * anima)));
        halo.setColorAt(1.0, QColor(255, 200, 120, 0));
        painter->fillRect(QRectF(a.x() - 110, a.y() - 110, 220, 220), halo);
        painter->setOpacity(anima);
        painter->drawPixmap(a - QPointF(spAnimaRamon.width() / 2.0, spAnimaRamon.height() / 2.0), spAnimaRamon);
        painter->setOpacity(1.0);
    }

    // Texto del bloque actual (aparece y se desvanece)
    int actual = 0;
    for (int i = 0; i < NUM_BLOQUES; ++i)
        if (tiempo >= BLOQUES[i].desde)
            actual = i;
    double desde = BLOQUES[actual].desde;
    double hasta = (actual + 1 < NUM_BLOQUES) ? BLOQUES[actual + 1].desde : FIN + 10.0;
    double a = rampa(tiempo, desde, desde + 0.8) * (1 - rampa(tiempo, hasta - 0.6, hasta));
    QFont f = painter->font();
    f.setPointSize(16);
    f.setItalic(true);
    painter->setFont(f);
    QString texto = QString::fromUtf8(BLOQUES[actual].texto);
    painter->setPen(QColor(0, 0, 0, int(200 * a)));
    painter->drawText(QRect(2, 72, 960, 90), Qt::AlignCenter, texto);
    painter->setPen(QColor(240, 234, 218, int(255 * a)));
    painter->drawText(QRect(0, 70, 960, 90), Qt::AlignCenter, texto);

    f.setItalic(false);
    f.setPointSize(10);
    painter->setFont(f);
    painter->setPen(QColor(170, 170, 160));
    QString ayuda = tiempo >= FIN - 0.5 ? QString::fromUtf8("Presiona Enter para huir")
                                        : QString::fromUtf8("Enter: continuar     Esc: saltar la introducción");
    if (tiempo >= FIN - 0.5) {
        f.setPointSize(14);
        f.setBold(true);
        painter->setFont(f);
        painter->setPen(QColor(250, 225, 160));
    }
    painter->drawText(QRect(0, 560, 960, 30), Qt::AlignCenter, ayuda);
    painter->restore();
}
