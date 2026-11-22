#ifndef CARLOS_H
#define CARLOS_H

#include "Personaje.h"
#include <QList>

class Rasgo;
class Nivel;
class Piedra;
class MovimientoFluido;

// Protagonista. Camina, corre, se esconde y lanza piedras.
// Sus rasgos (ConocedorDelMonte y Miedoso) ajustan en cada cuadro su
// visibilidad, velocidad, ruido y temblor. La fisica del agua llega en la fase 5.
class Carlos : public Personaje
{
public:
    explicit Carlos(const QPointF &posicion);
    ~Carlos() override;

    void actualizar(double dt) override;

    // --- Entrada del jugador ---
    void setDireccion(const QPointF &d) { direccion = d; }
    void correr(bool activo) { corriendo = activo; }
    // fuerza 0..1 (tiempo con el boton presionado). nullptr si aun no puede lanzar.
    Piedra *lanzarPiedra(double fuerza);
    bool esconderse();            // true si habia un escondite y entro
    void salirDelEscondite();

    // --- Panico (rasgo Miedoso) ---
    void aumentarPanico(double cantidad);
    void calmar(double cantidad);
    double ritmoLatidos() const;  // latidos por minuto segun el panico

    // --- Modificadores que aplican los rasgos (se reinician cada cuadro) ---
    void multiplicarVisibilidad(double f) { visibilidad *= f; }
    void multiplicarVelocidad(double f) { factorVelocidad *= f; }
    void multiplicarRuido(double f) { factorRuido *= f; }
    void setTemblor(double radianes) { temblor = radianes; }

    // --- Consultas ---
    bool estaCorriendo() const { return corriendo; }
    bool estaMoviendose() const;
    bool estaEscondido() const { return escondido; }
    bool puedeLanzar() const { return recargaPiedra <= 0.0 && !escondido; }
    double getTiempoEscondido() const { return tiempoEscondido; }
    double getPanico() const { return panico; }
    double getRuido() const { return ruido; }
    double getVisibilidad() const { return visibilidad; }
    double getTemblor() const { return temblor; }
    double getAire() const { return aire; }
    const QList<Rasgo *> &getRasgos() const { return rasgos; }

    void setNivel(Nivel *n) { nivel = n; }
    void setLimites(const QRectF &l) { limites = l; }

private:
    QPointF direccion;
    QRectF limites;
    Nivel *nivel;              // no es dueno; lo usa para que los rasgos consulten el terreno
    QList<Rasgo *> rasgos;     // memoria dinamica: Carlos es el dueno

    bool corriendo;
    bool escondido;
    double tiempoEscondido;
    double esperaEscondite;    // tiempo antes de poder volver a esconderse
    double recargaPiedra;

    double panico;             // 0..100
    double ruido;              // 0..1.5 (0 = silencio)
    double visibilidad;        // 0..1   (0 = invisible)
    double factorVelocidad;
    double factorRuido;
    double temblor;            // radianes de desvio al caminar
    double aire;               // 0..100 (fase 5)

    static constexpr double VELOCIDAD_CAMINAR = 140.0;   // px/s
    static constexpr double FACTOR_CORRER = 1.8;
    static constexpr double TIEMPO_RECARGA_PIEDRA = 1.2;
    static constexpr double ESPERA_ESCONDITE = 2.0;
};

#endif
