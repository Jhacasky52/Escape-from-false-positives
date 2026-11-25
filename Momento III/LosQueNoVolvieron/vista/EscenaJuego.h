#ifndef ESCENAJUEGO_H
#define ESCENAJUEGO_H

#include <QGraphicsScene>
#include <QTimer>
#include <QElapsedTimer>
#include <QSet>
#include <QString>

class Sonidos;
class QPainter;

// Escena base con el bucle de juego.
// - Un QTimer dispara cada ~16 ms (unos 60 cuadros por segundo).
// - La logica avanza con paso FIJO (DT = 1/60 s) usando un acumulador,
//   asi las fisicas dan el mismo resultado aunque el PC sea lento o rapido.
// - Despues de avanzar la logica se llama actualizarVista() para mover sprites.
// Cada nivel (o la escena de prueba) hereda de aqui.
class EscenaJuego : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit EscenaJuego(QObject *parent = nullptr);

    void iniciarBucle();
    void detenerBucle();
    double getFps() const { return fps; }

    void setSonidos(Sonidos *s) { sonidos = s; }
    // Mensaje corto que se ve unos segundos (por ejemplo "Partida guardada")
    void mostrarAviso(const QString &texto, double segundos = 2.5);

    static constexpr double DT = 1.0 / 60.0;

signals:
    // El jugador pidio seguir al siguiente nivel (Enter con el nivel superado)
    void siguienteNivel();
    void guardarPartida();   // G en la pausa
    void volverAlMenu();     // M en la pausa o al perder

protected:
    virtual void actualizarLogica(double dt) = 0;
    virtual void actualizarVista() = 0;

    bool teclaPresionada(int tecla) const { return teclas.contains(tecla); }
    void sonar(const QString &nombre, double volumen = 1.0);   // no hace nada sin Sonidos
    void dibujarAviso(QPainter *painter, int ancho, int alto);  // en coordenadas de pantalla

    Sonidos *sonidos;

    void keyPressEvent(QKeyEvent *evento) override;
    void keyReleaseEvent(QKeyEvent *evento) override;
    // Para teclas de una sola pulsacion (pausa, lanzar...)
    virtual void teclaPulsada(int tecla) { Q_UNUSED(tecla); }
    virtual void teclaSoltada(int tecla) { Q_UNUSED(tecla); }

private slots:
    void tick();

private:
    QTimer temporizador;
    QElapsedTimer reloj;
    double acumulador;
    double fps;
    int cuadros;
    double tiempoFps;
    QSet<int> teclas;
    QString aviso;
    double tiempoAviso;
};

#endif
