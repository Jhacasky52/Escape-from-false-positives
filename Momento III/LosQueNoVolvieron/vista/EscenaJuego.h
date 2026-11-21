#ifndef ESCENAJUEGO_H
#define ESCENAJUEGO_H

#include <QGraphicsScene>
#include <QTimer>
#include <QElapsedTimer>
#include <QSet>

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

    static constexpr double DT = 1.0 / 60.0;

protected:
    virtual void actualizarLogica(double dt) = 0;
    virtual void actualizarVista() = 0;

    bool teclaPresionada(int tecla) const { return teclas.contains(tecla); }

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
};

#endif
