#ifndef JUEGO_H
#define JUEGO_H

#include <QVector>

class Nivel;

enum class EstadoJuego {
    JUGANDO,
    PAUSADO,
    NIVEL_SUPERADO,
    CAPTURADO,
    FINAL
};

// Controla el flujo general: niveles, estado y (fase 4) la memoria de las animas.
// Es dueno de los niveles y los borra en su destructor.
class Juego
{
public:
    Juego();
    ~Juego();

    void agregarNivel(Nivel *nivel);   // Juego pasa a ser el dueno
    void iniciar();
    void actualizar(double dt);
    void pasarAlSiguienteNivel();
    void reiniciarNivel();
    void alternarPausa();

    Nivel *nivelActual() const;
    int getIndiceNivel() const { return indiceNivel; }
    EstadoJuego getEstado() const { return estado; }

private:
    QVector<Nivel *> niveles;
    int indiceNivel;
    EstadoJuego estado;
    // Aprendizaje *memoriaAnimas;   // fase 4: persiste al reiniciar el nivel
};

#endif
