#ifndef JUEGO_H
#define JUEGO_H

#include <QVector>

class Nivel;
class Aprendizaje;

enum class EstadoJuego {
    JUGANDO,
    PAUSADO,
    NIVEL_SUPERADO,
    CAPTURADO,
    FINAL
};

// Controla el flujo general: niveles, estado y la memoria de las animas.
// Es dueno de los niveles y los borra en su destructor.
class Juego
{
public:
    Juego();
    ~Juego();

    void agregarNivel(Nivel *nivel);   // Juego pasa a ser el dueno
    void iniciar();                     // empieza en el primer nivel
    void iniciarEn(int indice);         // empieza en el nivel 'indice' (al cargar la partida)
    void nuevaPartida();                // borra lo aprendido por las animas y empieza de cero
    void actualizar(double dt);
    void pasarAlSiguienteNivel();
    void reiniciarNivel();
    void alternarPausa();

    Nivel *nivelActual() const;
    Aprendizaje *getMemoriaAnimas() const { return memoriaAnimas; }
    int getIndiceNivel() const { return indiceNivel; }
    int numeroNiveles() const { return niveles.size(); }
    EstadoJuego getEstado() const { return estado; }

private:
    QVector<Nivel *> niveles;
    int indiceNivel;
    EstadoJuego estado;
    Aprendizaje *memoriaAnimas;   // lo que aprenden las animas: persiste al reiniciar
};

#endif
