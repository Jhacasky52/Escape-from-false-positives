#include "Sonidos.h"
#ifdef HAY_MULTIMEDIA
#include <QSoundEffect>
#include <QUrl>
#endif

Sonidos::Sonidos(QObject *parent) : QObject(parent) {}

QSoundEffect *Sonidos::efecto(const QString &nombre)
{
#ifdef HAY_MULTIMEDIA
    if (!efectos.contains(nombre)) {
        QSoundEffect *e = new QSoundEffect(this);
        e->setSource(QUrl("qrc:/sonidos/" + nombre + ".wav"));
        efectos[nombre] = e;
    }
    return efectos[nombre];
#else
    Q_UNUSED(nombre);
    return nullptr;
#endif
}

void Sonidos::reproducir(const QString &nombre, double volumen)
{
#ifdef HAY_MULTIMEDIA
    QSoundEffect *e = efecto(nombre);
    e->setVolume(qBound(0.0, volumen, 1.0));
    e->play();
#else
    Q_UNUSED(nombre);
    Q_UNUSED(volumen);
#endif
}

void Sonidos::ambiente(const QString &nombre, double volumen)
{
#ifdef HAY_MULTIMEDIA
    if (nombre == ambienteActual)
        return;
    if (!ambienteActual.isEmpty())
        efecto(ambienteActual)->stop();
    ambienteActual = nombre;
    if (nombre.isEmpty())
        return;
    QSoundEffect *e = efecto(nombre);
    e->setLoopCount(QSoundEffect::Infinite);
    e->setVolume(qBound(0.0, volumen, 1.0));
    e->play();
#else
    ambienteActual = nombre;
    Q_UNUSED(volumen);
#endif
}
