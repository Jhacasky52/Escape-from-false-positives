#ifndef PARTIDA_H
#define PARTIDA_H

#include <QString>

class Juego;

// Guarda y carga la partida en un archivo JSON:
//  - el nivel al que llego Carlos
//  - lo que aprendieron las animas (mapa de peligro, capturas y obediencia)
// Ejemplo:
//   { "juego": "Los que no volvieron", "version": 1, "nivel": 1, "dificultad": 2,
//     "guardado": "2026-11-24T19:30:00",
//     "animas": { "obediencia": 0.82, "capturas": [[12, 7]],
//                 "peligro": [[12, 7, 1.5], [13, 7, 0.4]] } }
class Partida
{
public:
    static bool guardar(const Juego &juego, const QString &ruta);
    // Deja a 'juego' en el nivel guardado y con la memoria de las animas.
    // Devuelve false (y no toca el juego) si el archivo no existe o esta danado.
    static bool cargar(Juego &juego, const QString &ruta);

    static bool existe(const QString &ruta);
    static bool borrar(const QString &ruta);
    // Carpeta de datos de la aplicacion del usuario (por ejemplo AppData en Windows)
    static QString rutaPorDefecto();

    static constexpr int VERSION = 1;
};

#endif
