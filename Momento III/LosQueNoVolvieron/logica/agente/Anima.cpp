#include "Anima.h"
#include "Percepcion.h"
#include "Razonamiento.h"
#include "Accion.h"
#include "Aprendizaje.h"
#include "fisica/MovimientoOscilatorio.h"
#include "niveles/Nivel.h"
#include "entidades/Carlos.h"
#include <QtMath>

Anima::Anima(const QPointF &posicion, Aprendizaje *memoria, int indice)
    : posicion(posicion), color(ColorAnima::BLANCA), brillo(0.8), brilloMaximo(1.0), tiempo(0.0),
      vaiven(new MovimientoOscilatorio(7.0, 2.0 + 0.35 * indice, indice * 1.7)),
      percepcion(new Percepcion()), razonamiento(new Razonamiento()), accion(new Accion()),
      aprendizaje(memoria), esperaDecision(0.0), esperaObediencia(INTERVALO_OBEDIENCIA)
{
    double angulo = indice * 2.0 * M_PI / 3.0;
    desplazamiento = QPointF(qCos(angulo), qSin(angulo)) * 22.0;
}

Anima::~Anima()
{
    delete vaiven;
    delete percepcion;
    delete razonamiento;
    delete accion;
}

QPointF Anima::posicionDibujo() const
{
    return posicion + vaiven->paso(0.0);
}

void Anima::actualizar(double dt, Nivel *nivel)
{
    tiempo += dt;
    vaiven->paso(dt);
    Carlos *carlos = nivel->getCarlos();
    if (carlos == nullptr)
        return;

    // 4. Aprendizaje: ¿Carlos siguio la ultima indicacion?
    esperaObediencia -= dt;
    if (esperaObediencia <= 0.0) {
        evaluarObediencia(nivel);
        esperaObediencia = INTERVALO_OBEDIENCIA;
    }

    esperaDecision -= dt;
    if (esperaDecision <= 0.0) {
        esperaDecision = INTERVALO_DECISION;
        // 1. Percepcion
        Estimulo e = percepcion->percibir(nivel, posicion);
        if (aprendizaje != nullptr)
            for (const QPointF &s : e.posSoldados)
                aprendizaje->registrarAvistamiento(nivel->celdaDe(s), INTERVALO_DECISION);
        // 2. Razonamiento
        decision = razonamiento->decidir(e, aprendizaje);
    }

    // 3. Accion (cada cuadro, para que el movimiento sea suave)
    double insistencia = aprendizaje != nullptr ? aprendizaje->insistencia() : 0.0;
    accion->ejecutar(decision, this, carlos, insistencia, dt);
    brillo = qMin(brillo, brilloMaximo);
}

void Anima::setAyuda(double radioPercepcion, double umbralAdvertencia, double brilloMax)
{
    percepcion->setRadio(radioPercepcion);
    razonamiento->setUmbralRiesgo(umbralAdvertencia);
    brilloMaximo = qBound(0.0, brilloMax, 1.0);
}

void Anima::evaluarObediencia(Nivel *nivel)
{
    Carlos *carlos = nivel->getCarlos();
    QPointF ahora = carlos->getPosicion();

    bool indicabaCamino = decisionEvaluada.tipo == TipoDecision::GUIAR
                       || decisionEvaluada.tipo == TipoDecision::ADVERTIR;
    QPointF movido = ahora - carlosAlIndicar;
    double distancia = qSqrt(movido.x() * movido.x() + movido.y() * movido.y());

    if (aprendizaje != nullptr && indicabaCamino && distancia > 24.0) {
        QPointF indicado = decisionEvaluada.destino - carlosAlIndicar;
        double largoIndicado = qSqrt(indicado.x() * indicado.x() + indicado.y() * indicado.y());
        if (largoIndicado > 1.0) {
            // Coseno del angulo entre lo que hizo y lo que se le indico
            double coseno = (movido.x() * indicado.x() + movido.y() * indicado.y())
                            / (distancia * largoIndicado);
            aprendizaje->registrarRespuesta(decisionEvaluada, coseno > 0.3);
        }
    }
    carlosAlIndicar = ahora;
    decisionEvaluada = decision;
}
