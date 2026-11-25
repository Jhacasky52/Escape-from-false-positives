#ifndef SONIDOS_H
#define SONIDOS_H

#include <QObject>
#include <QMap>
#include <QString>

class QSoundEffect;

// Todos los sonidos del juego (recursos/sonidos/*.wav, dentro del ejecutable).
// Usa Qt Multimedia si esta instalado; si no, los metodos no hacen nada y el
// juego funciona igual en silencio.
//  - reproducir("pasos", 0.4): efecto corto
//  - ambiente("ambiente_rio"): sonido de fondo que se repite (solo uno a la vez)
class Sonidos : public QObject
{
    Q_OBJECT
public:
    explicit Sonidos(QObject *parent = nullptr);

    void reproducir(const QString &nombre, double volumen = 1.0);
    void ambiente(const QString &nombre, double volumen = 0.6);   // "" = silencio
    void detenerAmbiente() { ambiente(QString()); }

private:
    QSoundEffect *efecto(const QString &nombre);

    QMap<QString, QSoundEffect *> efectos;   // hijos de este QObject (Qt los borra)
    QString ambienteActual;
};

#endif
