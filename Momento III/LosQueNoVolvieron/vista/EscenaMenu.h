#ifndef ESCENAMENU_H
#define ESCENAMENU_H

#include "EscenaJuego.h"
#include <QPixmap>
#include <QStringList>

// Menu principal: Nueva partida, Continuar (si hay partida guardada), Controles y Salir.
// Se maneja con W/S o flechas y Enter. No tiene logica del juego: solo avisa
// con senales a VentanaPrincipal lo que eligio el jugador.
class EscenaMenu : public EscenaJuego
{
    Q_OBJECT
public:
    explicit EscenaMenu(QObject *parent = nullptr);

    void setHayPartida(bool hay);

signals:
    void nuevaPartida();
    void continuarPartida();
    void salir();

protected:
    void actualizarLogica(double dt) override;
    void actualizarVista() override;
    void teclaPulsada(int tecla) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    enum Opcion { NUEVA, CONTINUAR, CONTROLES, SALIR, NUM_OPCIONES };
    void mover(int paso);

    int seleccion;
    bool hayPartida;
    bool viendoControles;
    double tiempo;
    QPixmap fondo;
    QPixmap spAnima, spRamon;
};

#endif
