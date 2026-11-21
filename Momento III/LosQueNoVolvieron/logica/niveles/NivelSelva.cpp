#include "NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QtMath>

NivelSelva::NivelSelva(const QString &rutaMapa)
    : rutaMapa(rutaMapa), columnas(0), filas(0) {}

// ------------------------------------------------------------ Carga

void NivelSelva::leerMapa()
{
    celdas.clear();
    rutasSoldados.clear();
    columnas = filas = 0;

    QFile archivo(rutaMapa);
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("No se pudo abrir el mapa %s", qPrintable(rutaMapa));
        return;
    }

    QTextStream entrada(&archivo);
    QStringList lineasMapa;
    while (!entrada.atEnd()) {
        QString linea = entrada.readLine();
        if (linea.startsWith('#') || linea.trimmed().isEmpty())
            continue;
        if (linea.startsWith("RUTA")) {
            QVector<QPointF> ruta;
            const QStringList puntos = linea.mid(4).split(' ', Qt::SkipEmptyParts);
            for (const QString &p : puntos) {
                QStringList xy = p.split(',');
                if (xy.size() == 2)
                    ruta.append(centroCelda(xy[0].toInt(), xy[1].toInt()));
            }
            rutasSoldados.append(ruta);
        } else {
            lineasMapa.append(linea);
        }
    }

    filas = lineasMapa.size();
    columnas = filas > 0 ? lineasMapa.first().size() : 0;
    celdas.resize(columnas * filas);

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            QChar ch = c < lineasMapa[f].size() ? lineasMapa[f][c] : QChar('.');
            Celda tipo = Celda::SUELO;
            switch (ch.toLatin1()) {
            case '=': tipo = Celda::TROCHA; break;
            case 'T': tipo = Celda::ARBOL; break;
            case 'M': tipo = Celda::MATORRAL; break;
            case 'H': tipo = Celda::ESCONDITE; break;
            case '~': tipo = Celda::RIO; break;
            case 'C': tipo = Celda::TROCHA; inicioCarlos = centroCelda(c, f); break;
            default: break;
            }
            celdas[f * columnas + c] = tipo;
        }
    }

    // La meta es la franja del rio (desde la primera columna con '~')
    int primeraColumnaRio = columnas;
    for (int c = 0; c < columnas; ++c)
        if (celdaEn(c, filas / 2) == Celda::RIO) { primeraColumnaRio = c; break; }
    meta = QRectF(primeraColumnaRio * TAM_CELDA, 0,
                  (columnas - primeraColumnaRio) * TAM_CELDA, filas * TAM_CELDA);
    limites = QRectF(0, 0, columnas * TAM_CELDA, filas * TAM_CELDA);
}

void NivelSelva::cargar()
{
    liberar();
    capturado = false;
    if (celdas.isEmpty())
        leerMapa();

    carlos = new Carlos(inicioCarlos);
    carlos->setLimites(limites.adjusted(16, 16, -16, -16));

    for (const QVector<QPointF> &ruta : rutasSoldados)
        soldados.append(new Soldado(ruta));
}

// ------------------------------------------------------------ Actualizacion

void NivelSelva::actualizar(double dt)
{
    if (carlos == nullptr)
        return;

    QPointF anterior = carlos->getPosicion();
    carlos->actualizar(dt);
    moverConColision(carlos, anterior);

    auto luz = [this](const QPointF &p) { return bloqueaLuz(p); };

    for (Soldado *s : soldados) {
        QPointF antes = s->getPosicion();
        s->actualizar(dt);
        moverConColision(s, antes);

        s->detecta(carlos, dt, luz);

        // Lo atrapan si un soldado que lo persigue lo alcanza
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO && s->colisionaCon(carlos))
            capturado = true;
    }
}

// Si el personaje quedo dentro de un arbol, se intenta mover solo en x o
// solo en y (asi "resbala" por el borde); si tampoco se puede, vuelve atras.
void NivelSelva::moverConColision(Personaje *p, const QPointF &anterior)
{
    if (!esSolido(p->caja()))
        return;
    QPointF nueva = p->getPosicion();

    p->setPosicion(QPointF(nueva.x(), anterior.y()));
    if (!esSolido(p->caja()))
        return;
    p->setPosicion(QPointF(anterior.x(), nueva.y()));
    if (!esSolido(p->caja()))
        return;
    p->setPosicion(anterior);
}

// ------------------------------------------------------------ Consultas

QPointF NivelSelva::centroCelda(int columna, int fila) const
{
    return QPointF((columna + 0.5) * TAM_CELDA, (fila + 0.5) * TAM_CELDA);
}

Celda NivelSelva::celdaEn(int columna, int fila) const
{
    if (columna < 0 || fila < 0 || columna >= columnas || fila >= filas)
        return Celda::ARBOL;   // fuera del mapa = no se puede pasar
    return celdas[fila * columnas + columna];
}

Celda NivelSelva::celdaEnPunto(const QPointF &p) const
{
    return celdaEn(qFloor(p.x() / TAM_CELDA), qFloor(p.y() / TAM_CELDA));
}

bool NivelSelva::esSolido(const QRectF &caja) const
{
    // El tronco del arbol ocupa el centro de la celda (40x40 de 64x64)
    const double margen = 12.0;
    int c0 = qFloor(caja.left() / TAM_CELDA), c1 = qFloor(caja.right() / TAM_CELDA);
    int f0 = qFloor(caja.top() / TAM_CELDA), f1 = qFloor(caja.bottom() / TAM_CELDA);
    for (int f = f0; f <= f1; ++f) {
        for (int c = c0; c <= c1; ++c) {
            if (celdaEn(c, f) != Celda::ARBOL)
                continue;
            QRectF tronco(c * TAM_CELDA + margen, f * TAM_CELDA + margen,
                          TAM_CELDA - 2 * margen, TAM_CELDA - 2 * margen);
            if (tronco.intersects(caja))
                return true;
        }
    }
    return false;
}

bool NivelSelva::bloqueaLuz(const QPointF &p) const
{
    return celdaEnPunto(p) == Celda::ARBOL;
}

bool NivelSelva::carlosEnVegetacion() const
{
    if (carlos == nullptr)
        return false;
    Celda c = celdaEnPunto(carlos->getPosicion());
    return c == Celda::MATORRAL || c == Celda::ESCONDITE;
}

bool NivelSelva::algunSoldadoPersigue() const
{
    for (const Soldado *s : soldados)
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO)
            return true;
    return false;
}
