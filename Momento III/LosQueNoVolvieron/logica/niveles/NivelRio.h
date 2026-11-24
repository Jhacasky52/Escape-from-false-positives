#ifndef NIVELRIO_H
#define NIVELRIO_H

#include "Nivel.h"
#include <QVector>
#include <QList>

class Tronco;

// Lugar seco donde Carlos puede pararse a descansar y respirar
enum class TipoApoyo { ORILLA, ROCA_ALTA, ROCA_BAJA, BANCO_ARENA };

struct Apoyo {
    QRectF zona;      // top() = donde pisa; el cuerpo de la roca va hacia abajo
    TipoApoyo tipo;
};

enum class EtapaRio { NORMAL, MEDIA, CRECIENTE };
enum class MotivoDerrota { NINGUNO, VISTO, AHOGADO, ARRASTRADO };

// Nivel 2: el rio, visto de lado. Carlos debe cruzarlo rio arriba (hacia la
// derecha) hasta la orilla de la salida mientras la corriente lo empuja hacia
// la izquierda (MovimientoFluido).
//  - Rocas y bancos de arena: puntos de apoyo para descansar y respirar.
//  - Bucear (abajo) lo esconde de las linternas pero gasta aire.
//  - Troncos y escombros bajan con la corriente (MovimientoOscilatorio) y lo empujan.
//  - Soldados en la orilla barren el agua con linternas (MovimientoPendular).
//  - Cada CICLO_CRECIENTE segundos el rio crece: sube el agua (tapa rocas bajas
//    y bancos) y la corriente se vuelve mas fuerte.
// Pierde si lo ven, si se ahoga o si la corriente lo saca de la pantalla.
class NivelRio : public Nivel
{
public:
    NivelRio();
    ~NivelRio() override;

    void cargar() override;
    void actualizar(double dt) override;

    // Consultas del terreno
    bool esEscondite(const QPointF &p) const override;
    bool esTransitable(const QPointF &p) const override;
    double corrienteEn(const QPointF &p) const override;
    double tiempoParaCreciente() const override;
    bool refugioCercano(const QPointF &p, QPointF &refugio) const override;

    // Estado del rio
    double getSuperficie() const { return superficie; }
    double getVelocidadRio() const { return velocidadRio; }
    EtapaRio getEtapa() const;
    double getTiempo() const { return tiempo; }
    bool apoyoSeco(const Apoyo &a) const;
    int apoyoBajo(const QPointF &pie) const;   // indice del apoyo que pisa (-1 si ninguno)

    const QVector<Apoyo> &getApoyos() const { return apoyos; }
    const QList<Tronco *> &getTroncos() const { return troncos; }
    MotivoDerrota getMotivo() const { return motivo; }
    double getLimiteArrastre() const { return avanceMaximo - DISTANCIA_ARRASTRE; }
    int getGolpes() const { return golpes; }
    int getApoyoActual() const { return apoyoActual; }

    // Para las pruebas: fija el momento del ciclo de la creciente
    void setTiempo(double t);
    void agregarTronco(Tronco *t);
    void retirarSoldados();          // solo para las pruebas
    void setGenerarTroncos(bool g);  // idem

    static constexpr double LARGO = 4800.0;
    static constexpr double ALTO = 600.0;
    static constexpr double FONDO = 560.0;           // lecho del rio
    static constexpr double Y_ORILLA_SOLDADOS = 200.0;
    static constexpr double SUPERFICIE_NORMAL = 300.0;
    static constexpr double SUPERFICIE_CRECIENTE = 250.0;
    static constexpr double CICLO_CRECIENTE = 45.0;  // s
    static constexpr double INICIO_SUBIDA = 20.0;    // s del ciclo en que empieza a crecer
    static constexpr double FIN_SUBIDA = 28.0;
    static constexpr double INICIO_BAJADA = 36.0;
    static constexpr double DISTANCIA_ARRASTRE = 560.0;
    static constexpr double MEDIA_ALTURA_CARLOS = 14.0;

private:
    void crearApoyos();
    void actualizarAgua(double dt);
    void actualizarCarlos(double dt);
    void actualizarTroncos(double dt);
    void actualizarSoldados(double dt);
    void liberarTroncos();
    double aleatorio();   // 0..1, generador propio para que las pruebas sean repetibles

    QVector<Apoyo> apoyos;
    QList<Tronco *> troncos;   // memoria dinamica: el nivel es el dueno
    double tiempo;
    double superficie;
    double velocidadRio;
    double avanceMaximo;       // lo mas lejos que ha llegado Carlos
    double proximoTronco;
    double esperaRuido;
    double esperaGolpe;
    int golpes;
    int apoyoActual;           // donde esta parado Carlos (-1 = en el agua)
    int contadorTroncos;
    bool generarTroncos;
    unsigned int semilla;
    MotivoDerrota motivo;
};

#endif
