#ifndef DIFICULTAD_H
#define DIFICULTAD_H

#include <QString>

enum class NivelDificultad { FACIL, NORMAL, DIFICIL };

// Abstraccion de la dificultad (se aplica al nivel 1, la selva).
// La dificultad NO es "mas soldados" ni "menos tiempo": es cuanto sabe cada lado
// y cuanto se perdona un error. Se separa en cuatro ejes:
//
//  1. Percepcion del enemigo  -> que tan rapido y lejos detectan los soldados
//  2. Inteligencia del enemigo -> COMO actuan: si se avisan entre ellos, si
//                                 buscan hacia donde iba Carlos, si revisan escondites
//  3. Ayuda de las animas     -> cuanta informacion recibe el jugador
//  4. Margen de error         -> cuanto aguanta Carlos (panico y escondite)
//
// Cada eje cambia el comportamiento de una clase distinta (Soldado, Anima,
// Miedoso, ConocedorDelMonte); NivelSelva los aplica al cargar.
struct Dificultad
{
    NivelDificultad nivel = NivelDificultad::NORMAL;

    // 1. Percepcion del enemigo
    double tiempoParaDescubrir = 0.7;   // s a plena luz hasta reconocer a Carlos
    double factorOido = 1.0;            // multiplica el radio en que oye los ruidos
    double tiempoParaRendirse = 4.0;    // s sin verlo antes de dejar la persecucion
    double olvidoSospecha = 0.5;        // cuanto baja la sospecha por segundo sin verlo

    // 2. Inteligencia del enemigo (comportamientos)
    double radioAviso = 0.0;            // px: los soldados a esta distancia acuden (0 = no se avisan)
    bool busquedaPredictiva = false;    // al perderlo, busca hacia donde iba Carlos
    bool revisaEscondites = false;      // al investigar, revisa los escondites cercanos

    // 3. Ayuda de las animas
    double radioPercepcionAnimas = 520.0;  // px que perciben
    double umbralAdvertencia = 0.75;       // riesgo desde el que advierten (mas bajo = avisan antes)
    double brilloMaximoAnimas = 1.0;       // con menos brillo cuesta mas verlas
    double periodoRamon = 30.0;            // s entre apariciones de Ramon

    // 4. Margen de error
    double umbralPanico = 60.0;         // panico desde el que Carlos tiembla y hace ruido
    double tiempoMaxEscondido = 6.0;    // s que aguanta dentro de un escondite

    static Dificultad crear(NivelDificultad nivel);
    static QString nombre(NivelDificultad nivel);
    QString resumen() const;   // texto corto para el modo depuracion (F1)
};

#endif
