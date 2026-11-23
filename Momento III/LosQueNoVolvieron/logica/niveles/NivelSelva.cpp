#include "NivelSelva.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Piedra.h"
#include "agente/Anima.h"
#include "agente/AnimaRamon.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QtMath>
#include <QQueue>

NivelSelva::NivelSelva(const QString &rutaMapa)
    : rutaMapa(rutaMapa), columnas(0), filas(0), temporizadorPasos(0.0) {}

NivelSelva::~NivelSelva()
{
    qDeleteAll(piedras);
}

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
    qDeleteAll(piedras);
    piedras.clear();
    capturado = false;
    temporizadorPasos = 0.0;
    if (celdas.isEmpty())
        leerMapa();

    carlos = new Carlos(inicioCarlos);
    carlos->setLimites(limites.adjusted(16, 16, -16, -16));
    carlos->setNivel(this);

    Planificador planificador = [this](const QPointF &a, const QPointF &b) {
        return buscarCamino(a, b);
    };
    for (const QVector<QPointF> &ruta : rutasSoldados) {
        Soldado *s = new Soldado(ruta);
        s->setPlanificador(planificador);
        soldados.append(s);
    }

    // Tres animas de victimas y la de Ramon acompanan a Carlos desde el inicio
    for (int i = 0; i < 3; ++i)
        animas.append(new Anima(inicioCarlos + QPointF(40, -30 + 30 * i), memoria, i));
    animas.append(new AnimaRamon(inicioCarlos + QPointF(-30, 0), memoria));
}

// ------------------------------------------------------------ Actualizacion

void NivelSelva::actualizar(double dt)
{
    if (carlos == nullptr)
        return;

    QPointF anterior = carlos->getPosicion();
    carlos->actualizar(dt);
    moverConColision(carlos, anterior);

    // Los pasos de Carlos suenan: caminando apenas, corriendo (o con panico) mucho
    temporizadorPasos += dt;
    if (temporizadorPasos >= 0.4) {
        temporizadorPasos = 0.0;
        if (carlos->getRuido() > 0.0)
            hacerRuido(carlos->getPosicion(), carlos->getRuido() * 0.55);
    }

    // Piedras en el aire: si chocan con un arbol caen ahi; al caer hacen ruido
    for (int i = piedras.size() - 1; i >= 0; --i) {
        Piedra *p = piedras[i];
        p->actualizar(dt);
        bool lejosDeCarlos = celdaEnPunto(p->getPosicion()) != celdaEnPunto(p->getOrigen())
                             || p->getAltura() > 0.0;
        if (!p->cayo() && p->getAltura() < 40.0 && lejosDeCarlos && bloqueaLuz(p->getPosicion()))
            p->detener();
        if (p->cayo() && p->estaActiva()) {
            hacerRuido(p->getPosicion(), p->intensidadRuido());
            p->desactivar();
        }
    }
    // Solo se guardan las ultimas piedras caidas (para dibujarlas un rato)
    while (piedras.size() > 4 && !piedras.first()->estaActiva()) {
        delete piedras.takeFirst();
    }

    auto luz = [this](const QPointF &p) { return bloqueaLuz(p); };

    actualizarAnimas(dt);

    for (Soldado *s : soldados) {
        QPointF antes = s->getPosicion();
        s->actualizar(dt);
        moverConColision(s, antes);

        s->detecta(carlos, dt, luz);

        // Lo atrapan si un soldado que lo persigue lo alcanza (escondido se salva)
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO && !carlos->estaEscondido()
            && s->colisionaCon(carlos))
            capturado = true;
    }
}

bool NivelSelva::lanzarPiedra(double fuerza)
{
    if (carlos == nullptr)
        return false;
    Piedra *p = carlos->lanzarPiedra(fuerza);
    if (p == nullptr)
        return false;
    piedras.append(p);
    return true;
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

bool NivelSelva::esVegetacion(const QPointF &p) const
{
    Celda c = celdaEnPunto(p);
    return c == Celda::MATORRAL || c == Celda::ESCONDITE;
}

bool NivelSelva::esTrocha(const QPointF &p) const
{
    return celdaEnPunto(p) == Celda::TROCHA;
}

bool NivelSelva::esEscondite(const QPointF &p) const
{
    // Troncos huecos y matorrales sirven de escondite
    return esVegetacion(p);
}

bool NivelSelva::esTransitable(const QPointF &p) const
{
    return esTransitable(qFloor(p.x() / TAM_CELDA), qFloor(p.y() / TAM_CELDA));
}

bool NivelSelva::esTransitable(int columna, int fila) const
{
    Celda c = celdaEn(columna, fila);
    return c != Celda::ARBOL && c != Celda::RIO;
}

// Busqueda en anchura (BFS) sobre la cuadricula, con 8 vecinos y sin cortar
// esquinas de arboles. Devuelve los centros de las celdas del camino.
QVector<QPointF> NivelSelva::buscarCamino(const QPointF &desde, const QPointF &hasta) const
{
    QVector<QPointF> camino;
    int c0 = qFloor(desde.x() / TAM_CELDA), f0 = qFloor(desde.y() / TAM_CELDA);
    int c1 = qFloor(hasta.x() / TAM_CELDA), f1 = qFloor(hasta.y() / TAM_CELDA);
    if (!esTransitable(c1, f1) || columnas == 0)
        return camino;

    auto indice = [this](int c, int f) { return f * columnas + c; };
    QVector<int> anterior(columnas * filas, -2);   // -2 = no visitado
    QQueue<int> cola;
    anterior[indice(c0, f0)] = -1;
    cola.enqueue(indice(c0, f0));

    const int dc[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int df[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    bool encontrado = false;

    while (!cola.isEmpty()) {
        int actual = cola.dequeue();
        int c = actual % columnas, f = actual / columnas;
        if (c == c1 && f == f1) { encontrado = true; break; }
        for (int k = 0; k < 8; ++k) {
            int nc = c + dc[k], nf = f + df[k];
            if (!esTransitable(nc, nf))
                continue;
            // En diagonal, las dos celdas laterales tambien deben estar libres
            if (k >= 4 && (!esTransitable(c + dc[k], f) || !esTransitable(c, f + df[k])))
                continue;
            int vecino = indice(nc, nf);
            if (anterior[vecino] != -2)
                continue;
            anterior[vecino] = actual;
            cola.enqueue(vecino);
        }
    }
    if (!encontrado)
        return camino;

    // Reconstruye el camino de atras hacia adelante
    for (int i = indice(c1, f1); i != -1 && i != indice(c0, f0); i = anterior[i])
        camino.prepend(centroCelda(i % columnas, i / columnas));
    if (!camino.isEmpty())
        camino.last() = hasta;   // termina en el punto exacto
    return camino;
}

bool NivelSelva::algunSoldadoPersigue() const
{
    for (const Soldado *s : soldados)
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO)
            return true;
    return false;
}
