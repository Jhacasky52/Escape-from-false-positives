#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QMainWindow>

class QGraphicsView;
class EscenaJuego;
class Juego;

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
    void alternarPruebaFisica();

    QGraphicsView *vista;
    Juego *juego;
    EscenaJuego *escenaNivel1;
    EscenaJuego *escenaPrueba;
    EscenaJuego *escenaActual;
};

#endif
