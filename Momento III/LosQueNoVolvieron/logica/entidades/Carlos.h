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
// visibilidad, velocidad, ruido y temblor.
// En el rio (nivel 2) nada con MovimientoFluido: la corriente lo arrastra,
// puede sumergirse para esconderse de las linternas pero gasta aire.
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

    // --- Rio (fase 5) ---
    void setVistaLateral(bool v) { vistaLateral = v; }   // en tierra solo camina en x
    void entrarAlAgua();
    void salirDelAgua(const QPointF &pie);   // se para en una roca u orilla
    void setAgua(double superficie, double velocidadRio);
    void empujar(const QPointF &v);          // golpe de un tronco
    void setAire(double a) { aire = qBound(0.0, a, 100.0); }
    bool estaEnAgua() const { return enAgua; }
    bool estaSumergido() const { return enAgua && profundidad() > PROF_OCULTO; }
    double profundidad() const { return posicion.y() - superficie; }
    bool seAhogo() const { return aire <= 0.0; }
    const MovimientoFluido *getFluido() const { return fluido; }
    QPointF getDireccion() const { return direccion; }

    static constexpr double PROF_OCULTO = 30.0;     // px bajo la superficie: ya no lo ven

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
    double aire;               // 0..100: baja sumergido, sube al respirar

    // Rio
    void nadar(double dt);
    bool vistaLateral;
    bool enAgua;
    MovimientoFluido *fluido;  // memoria dinamica: Carlos es el dueno
    QPointF desplazamientoAgua;// ultimo valor de fluido->paso()
    double superficie;
    double velocidadRio;

    static constexpr double VELOCIDAD_CAMINAR = 140.0;   // px/s
    static constexpr double FACTOR_CORRER = 1.8;
    static constexpr double TIEMPO_RECARGA_PIEDRA = 1.2;
    static constexpr double ESPERA_ESCONDITE = 2.0;
    // Agua: m dv/dt = F + k (v_rio - v) - b v   (m = 1, k = 3, b = 1)
    static constexpr double MASA = 1.0;
    static constexpr double K_CORRIENTE = 3.0;
    static constexpr double B_RESISTENCIA = 1.0;
    static constexpr double FUERZA_NADO = 480.0;    // brazadas hacia adelante o atras
    static constexpr double FUERZA_BUCEO = 520.0;   // hacia el fondo
    static constexpr double K_FLOTACION = 9.0;      // lo devuelve a la superficie
    static constexpr double GASTO_AIRE = 12.5;      // por segundo sumergido (8 s de aire)
    static constexpr double RECUPERA_AIRE = 30.0;   // por segundo respirando
};

#endif
