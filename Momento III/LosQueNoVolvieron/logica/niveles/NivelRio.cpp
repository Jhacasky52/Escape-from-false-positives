#include "NivelRio.h"
#include "entidades/Carlos.h"
#include "entidades/Soldado.h"
#include "entidades/Linterna.h"
#include "entidades/Tronco.h"
#include "agente/Anima.h"
#include "agente/AnimaRamon.h"
#include <QtMath>
#include <cmath>

namespace {
double largo(const QPointF &d) { return qSqrt(d.x() * d.x() + d.y() * d.y()); }
}

NivelRio::NivelRio()
    : tiempo(0.0), superficie(SUPERFICIE_NORMAL), velocidadRio(-70.0), avanceMaximo(0.0),
      proximoTronco(0.0), esperaRuido(0.0), esperaGolpe(0.0), golpes(0), apoyoActual(-1),
      contadorTroncos(0), generarTroncos(true), semilla(12345u), motivo(MotivoDerrota::NINGUNO) {}

NivelRio::~NivelRio()
{
    liberarTroncos();
}

void NivelRio::liberarTroncos()
{
    qDeleteAll(troncos);
    troncos.clear();
}

double NivelRio::aleatorio()
{
    // Generador congruencial lineal (el mismo de muchas bibliotecas de C)
    semilla = semilla * 1103515245u + 12345u;
    return ((semilla >> 16) & 0x7FFF) / 32767.0;
}

// ------------------------------------------------------------ Carga

void NivelRio::crearApoyos()
{
    apoyos.clear();
    auto agregar = [this](double x1, double x2, double arriba, TipoApoyo tipo) {
        double alto = (tipo == TipoApoyo::ORILLA) ? FONDO - arriba + 40.0
                    : (tipo == TipoApoyo::BANCO_ARENA ? 40.0 : 80.0);
        apoyos.append({QRectF(x1, arriba, x2 - x1, alto), tipo});
    };
    agregar(0, 260, 250, TipoApoyo::ORILLA);              // de donde sale Carlos
    agregar(620, 730, 272, TipoApoyo::ROCA_BAJA);
    agregar(980, 1090, 236, TipoApoyo::ROCA_ALTA);
    agregar(1240, 1420, 292, TipoApoyo::BANCO_ARENA);
    agregar(1560, 1670, 272, TipoApoyo::ROCA_BAJA);
    agregar(1960, 2070, 272, TipoApoyo::ROCA_BAJA);
    agregar(2380, 2490, 236, TipoApoyo::ROCA_ALTA);
    agregar(2660, 2840, 292, TipoApoyo::BANCO_ARENA);
    agregar(2980, 3090, 272, TipoApoyo::ROCA_BAJA);
    agregar(3360, 3470, 272, TipoApoyo::ROCA_BAJA);
    agregar(3780, 3890, 236, TipoApoyo::ROCA_ALTA);
    agregar(4020, 4200, 292, TipoApoyo::BANCO_ARENA);
    agregar(4250, 4360, 272, TipoApoyo::ROCA_BAJA);
    agregar(4560, LARGO, 250, TipoApoyo::ORILLA);         // la otra orilla: la salida
}

void NivelRio::cargar()
{
    liberar();
    liberarTroncos();
    capturado = false;
    motivo = MotivoDerrota::NINGUNO;
    tiempo = 0.0;
    golpes = 0;
    contadorTroncos = 0;
    semilla = 12345u;
    proximoTronco = 3.0;
    esperaRuido = 0.0;
    esperaGolpe = 0.0;
    actualizarAgua(0.0);

    limites = QRectF(14, 20, LARGO - 28, FONDO - 20);
    meta = QRectF(4600, 0, LARGO - 4600, ALTO);
    crearApoyos();

    // Carlos sale parado en la orilla izquierda
    carlos = new Carlos(QPointF(120, apoyos[0].zona.top() - MEDIA_ALTURA_CARLOS));
    carlos->setNivel(this);
    carlos->setLimites(limites);
    carlos->setVistaLateral(true);
    carlos->setAgua(superficie, velocidadRio);
    apoyoActual = 0;
    avanceMaximo = carlos->getPosicion().x();

    // Soldados en la orilla de enfrente: la linterna apunta al agua
    const double rutas[3][2] = {{1150, 1750}, {2500, 3150}, {3550, 4250}};
    for (const auto &r : rutas) {
        QVector<QPointF> ruta = {QPointF(r[0], Y_ORILLA_SOLDADOS), QPointF(r[1], Y_ORILLA_SOLDADOS)};
        Soldado *s = new Soldado(ruta);
        s->fijarLinterna(M_PI / 2.0);
        s->configurarLinterna(300.0, qDegreesToRadians(22.0), qDegreesToRadians(30.0));
        double xMin = r[0] - 250.0, xMax = r[1] + 250.0;
        // Si oye algo va por la orilla hasta quedar encima del ruido (no se mete al agua)
        s->setPlanificador([xMin, xMax](const QPointF &, const QPointF &destino) {
            return QVector<QPointF>{QPointF(qBound(xMin, destino.x(), xMax), Y_ORILLA_SOLDADOS)};
        });
        soldados.append(s);
    }

    // Animas: descansan sobre las rocas altas; Ramon acompana desde el comienzo
    int i = 0;
    for (const Apoyo &a : apoyos)
        if (a.tipo == TipoApoyo::ROCA_ALTA)
            animas.append(new Anima(QPointF(a.zona.center().x(), a.zona.top() - 40), memoria, i++));
    animas.append(new AnimaRamon(QPointF(220, 200), memoria));
}

void NivelRio::setTiempo(double t)
{
    tiempo = t;
    actualizarAgua(0.0);
}

void NivelRio::agregarTronco(Tronco *t)
{
    if (t != nullptr) {
        t->setAgua(superficie, velocidadRio);
        troncos.append(t);
    }
}

void NivelRio::setGenerarTroncos(bool g)
{
    generarTroncos = g;
}

void NivelRio::retirarSoldados()
{
    qDeleteAll(soldados);
    soldados.clear();
}

// ------------------------------------------------------------ Bucle

void NivelRio::actualizar(double dt)
{
    if (carlos == nullptr || capturado)
        return;

    actualizarAgua(dt);
    actualizarCarlos(dt);
    if (capturado)
        return;
    actualizarTroncos(dt);
    actualizarAnimas(dt);
    actualizarSoldados(dt);
}

void NivelRio::actualizarAgua(double dt)
{
    tiempo += dt;
    double t = std::fmod(tiempo, CICLO_CRECIENTE);
    double alto = SUPERFICIE_NORMAL - SUPERFICIE_CRECIENTE;   // cuanto sube (50 px)
    if (t < INICIO_SUBIDA)
        superficie = SUPERFICIE_NORMAL;
    else if (t < FIN_SUBIDA)
        superficie = SUPERFICIE_NORMAL - alto * (t - INICIO_SUBIDA) / (FIN_SUBIDA - INICIO_SUBIDA);
    else if (t < INICIO_BAJADA)
        superficie = SUPERFICIE_CRECIENTE;
    else
        superficie = SUPERFICIE_CRECIENTE + alto * (t - INICIO_BAJADA) / (CICLO_CRECIENTE - INICIO_BAJADA);

    // Mas agua, mas corriente: -70 px/s normal, -150 px/s en la creciente
    velocidadRio = -(70.0 + (SUPERFICIE_NORMAL - superficie) * 1.6);
}

EtapaRio NivelRio::getEtapa() const
{
    if (superficie > SUPERFICIE_NORMAL - 10.0)
        return EtapaRio::NORMAL;
    if (superficie < SUPERFICIE_CRECIENTE + 10.0)
        return EtapaRio::CRECIENTE;
    return EtapaRio::MEDIA;
}

double NivelRio::tiempoParaCreciente() const
{
    double t = std::fmod(tiempo, CICLO_CRECIENTE);
    if (t < INICIO_SUBIDA)
        return INICIO_SUBIDA - t;
    if (t < INICIO_BAJADA)
        return 0.0;   // esta creciendo o ya crecio
    return CICLO_CRECIENTE - t + INICIO_SUBIDA;
}

bool NivelRio::apoyoSeco(const Apoyo &a) const
{
    // y crece hacia abajo: si la superficie esta por encima de donde se pisa, esta tapado
    return superficie >= a.zona.top() - 2.0;
}

int NivelRio::apoyoBajo(const QPointF &pie) const
{
    for (int i = 0; i < apoyos.size(); ++i) {
        const QRectF &z = apoyos[i].zona;
        if (pie.x() >= z.left() && pie.x() <= z.right())
            return i;
    }
    return -1;
}

void NivelRio::actualizarCarlos(double dt)
{
    carlos->setAgua(superficie, velocidadRio);
    carlos->actualizar(dt);
    QPointF p = carlos->getPosicion();

    if (!carlos->estaEnAgua()) {
        // En tierra: sigue parado mientras haya un apoyo seco bajo sus pies
        int i = apoyoBajo(p);
        if (i >= 0 && apoyoSeco(apoyos[i])) {
            carlos->setPosicion(QPointF(p.x(), apoyos[i].zona.top() - MEDIA_ALTURA_CARLOS));
            apoyoActual = i;
        } else {
            carlos->entrarAlAgua();   // se salio de la roca o el agua la tapo
            apoyoActual = -1;
        }
    } else {
        apoyoActual = -1;
        // En el agua: si se impulsa hacia arriba junto a un apoyo seco, se sube
        if (carlos->getDireccion().y() < 0.0) {
            for (int i = 0; i < apoyos.size(); ++i) {
                const Apoyo &a = apoyos[i];
                if (!apoyoSeco(a))
                    continue;
                const QRectF &z = a.zona;
                bool alLado = p.x() >= z.left() - 12.0 && p.x() <= z.right() + 12.0;
                bool alturaOk = p.y() >= z.top() - 40.0 && p.y() <= z.top() + 50.0;
                if (alLado && alturaOk) {
                    double x = qBound(z.left() + 4.0, p.x(), z.right() - 4.0);
                    carlos->salirDelAgua(QPointF(x, z.top() - MEDIA_ALTURA_CARLOS));
                    apoyoActual = i;
                    break;
                }
            }
        }
    }
    p = carlos->getPosicion();

    // Ruido de las brazadas o de los pasos sobre las rocas
    esperaRuido -= dt;
    if (esperaRuido <= 0.0 && carlos->getRuido() > 0.0) {
        hacerRuido(p, carlos->getRuido() * 0.7);
        esperaRuido = 0.5;
    }

    // Derrotas propias del rio
    avanceMaximo = qMax(avanceMaximo, p.x());
    if (carlos->seAhogo()) {
        motivo = MotivoDerrota::AHOGADO;
        capturado = true;
    } else if (p.x() < avanceMaximo - DISTANCIA_ARRASTRE) {
        motivo = MotivoDerrota::ARRASTRADO;
        capturado = true;
    }
}

void NivelRio::actualizarTroncos(double dt)
{
    // Aparecen rio arriba (a la derecha, fuera de la pantalla) y bajan con la corriente
    proximoTronco -= dt;
    double xNuevo = carlos->getPosicion().x() + 620.0;
    if (proximoTronco <= 0.0) {
        if (generarTroncos && xNuevo < meta.left() - 120.0) {
            TipoTronco tipo = (contadorTroncos % 3 == 2) ? TipoTronco::ESCOMBROS : TipoTronco::TRONCO;
            troncos.append(new Tronco(QPointF(xNuevo, superficie), tipo, aleatorio() * 6.28));
            ++contadorTroncos;
        }
        double factor = (getEtapa() == EtapaRio::NORMAL) ? 1.0 : 0.6;   // en la creciente bajan mas
        proximoTronco = (3.0 + aleatorio() * 3.0) * factor;
    }

    if (esperaGolpe > 0.0)
        esperaGolpe -= dt;
    for (int i = troncos.size() - 1; i >= 0; --i) {
        Tronco *t = troncos[i];
        t->setAgua(superficie, velocidadRio);
        t->actualizar(dt);
        // Un tronco solo golpea en la superficie: buceando se pasa por debajo
        if (esperaGolpe <= 0.0 && carlos->estaEnAgua() && carlos->profundidad() < 40.0
            && t->colisionaCon(carlos)) {
            carlos->empujar(QPointF(t->getVelocidad().x() * 1.6, 40.0));
            carlos->aumentarPanico(25.0);
            ++golpes;
            esperaGolpe = 1.0;
        }
        if (t->getPosicion().x() < carlos->getPosicion().x() - 900.0 || t->getPosicion().x() < -200.0) {
            delete t;
            troncos.removeAt(i);
        }
    }
}

void NivelRio::actualizarSoldados(double dt)
{
    // Las rocas tapan la luz (los bancos de arena son muy bajitos)
    auto bloqueaLuz = [this](const QPointF &p) {
        for (const Apoyo &a : apoyos)
            if ((a.tipo == TipoApoyo::ROCA_ALTA || a.tipo == TipoApoyo::ROCA_BAJA) && a.zona.contains(p))
                return true;
        return false;
    };
    for (Soldado *s : soldados) {
        s->actualizar(dt);
        s->detecta(carlos, dt, bloqueaLuz);
        if (s->getEstado() == EstadoSoldado::PERSIGUIENDO) {
            // En el rio no hay donde correr: si lo ven, lo atrapan
            motivo = MotivoDerrota::VISTO;
            capturado = true;
            return;
        }
    }
}

// ------------------------------------------------------------ Consultas

bool NivelRio::esEscondite(const QPointF &p) const
{
    // Bajo el agua (lo bastante hondo) las linternas no lo ven
    return p.y() > superficie + 2.0 * Carlos::PROF_OCULTO && p.y() < FONDO;
}

bool NivelRio::esTransitable(const QPointF &p) const
{
    return limites.contains(p) && p.y() > superficie - 70.0;
}

double NivelRio::corrienteEn(const QPointF &p) const
{
    return p.y() >= superficie - 2.0 ? velocidadRio : 0.0;
}

bool NivelRio::refugioCercano(const QPointF &p, QPointF &refugio) const
{
    // Rocas altas y orillas no se tapan con la creciente
    double mejor = -1.0;
    for (const Apoyo &a : apoyos) {
        if (a.tipo != TipoApoyo::ROCA_ALTA && a.tipo != TipoApoyo::ORILLA)
            continue;
        QPointF centro(qBound(a.zona.left() + 30.0, p.x(), a.zona.right() - 30.0),
                       a.zona.top() - 30.0);
        double d = largo(centro - p);
        if (mejor < 0.0 || d < mejor) {
            mejor = d;
            refugio = centro;
        }
    }
    return mejor >= 0.0;
}
