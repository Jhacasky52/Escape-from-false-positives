#include "Dificultad.h"

Dificultad Dificultad::crear(NivelDificultad nivel)
{
    Dificultad d;   // los valores por defecto son los de NORMAL (los del juego hasta la fase 6)
    d.nivel = nivel;
    switch (nivel) {
    case NivelDificultad::FACIL:
        d.tiempoParaDescubrir = 1.1;
        d.factorOido = 0.8;
        d.tiempoParaRendirse = 2.5;
        d.olvidoSospecha = 0.8;
        d.radioAviso = 0.0;
        d.busquedaPredictiva = false;
        d.revisaEscondites = false;
        d.radioPercepcionAnimas = 700.0;
        d.umbralAdvertencia = 0.55;
        d.brilloMaximoAnimas = 1.0;
        d.periodoRamon = 18.0;
        d.umbralPanico = 75.0;
        d.tiempoMaxEscondido = 8.0;
        break;
    case NivelDificultad::NORMAL:
        break;
    case NivelDificultad::DIFICIL:
        d.tiempoParaDescubrir = 0.5;
        d.factorOido = 1.25;
        d.tiempoParaRendirse = 6.0;
        d.olvidoSospecha = 0.3;
        d.radioAviso = 800.0;   // unas 12 celdas: el radio alcanza al soldado de la ruta vecina
        d.busquedaPredictiva = true;
        d.revisaEscondites = true;
        d.radioPercepcionAnimas = 300.0;
        d.umbralAdvertencia = 0.95;
        d.brilloMaximoAnimas = 0.55;
        d.periodoRamon = 45.0;
        d.umbralPanico = 45.0;
        d.tiempoMaxEscondido = 4.0;
        break;
    }
    return d;
}

QString Dificultad::nombre(NivelDificultad nivel)
{
    switch (nivel) {
    case NivelDificultad::FACIL:   return QString::fromUtf8("Fácil");
    case NivelDificultad::NORMAL:  return "Normal";
    case NivelDificultad::DIFICIL: return QString::fromUtf8("Difícil");
    }
    return "Normal";
}

QString Dificultad::resumen() const
{
    auto si = [](bool b) { return b ? QString::fromUtf8("sí") : QString("no"); };
    return QString::fromUtf8("Dificultad %1 | descubre en %2 s, oído x%3 | se avisan: %4, predice: %5, revisa escondites: %6 | ánimas %7 px | pánico desde %8")
        .arg(nombre(nivel))
        .arg(tiempoParaDescubrir, 0, 'f', 1)
        .arg(factorOido, 0, 'f', 2)
        .arg(si(radioAviso > 0.0)).arg(si(busquedaPredictiva)).arg(si(revisaEscondites))
        .arg(radioPercepcionAnimas, 0, 'f', 0)
        .arg(umbralPanico, 0, 'f', 0);
}
