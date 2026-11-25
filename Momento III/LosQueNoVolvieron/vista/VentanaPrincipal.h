#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QMainWindow>
#include <QString>

class QGraphicsView;
class EscenaJuego;
class EscenaMenu;
class EscenaNivelSelva;
class EscenaNivelRio;
class EscenaFinal;
class Juego;
class Sonidos;

// Ventana del juego. Arma la logica (Juego con sus dos niveles) y las escenas,
// y decide que escena se ve: menu -> selva -> rio -> final -> menu.
class VentanaPrincipal : public QMainWindow
{
    Q_OBJECT
public:
    explicit VentanaPrincipal(QWidget *parent = nullptr);
    ~VentanaPrincipal() override;

    static constexpr int ANCHO = 960;
    static constexpr int ALTO = 600;

private:
    void mostrarEscena(EscenaJuego *escena);
    EscenaJuego *escenaDelNivel() const;   // la escena del nivel actual
    void alternarPruebaFisica();

    // Respuestas a las escenas
    void irAlMenu();
    void nuevaPartida();
    void continuarPartida();
    void guardarPartida();
    void siguienteNivel();

    QGraphicsView *vista;
    Juego *juego;
    Sonidos *sonidos;
    QString rutaPartida;
    EscenaMenu *escenaMenu;
    EscenaNivelSelva *escenaNivel1;
    EscenaNivelRio *escenaNivel2;
    EscenaFinal *escenaFinal;
    EscenaJuego *escenaPrueba;
    EscenaJuego *escenaActual;
};

#endif
