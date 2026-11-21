#ifndef NIVEL_H
#define NIVEL_H

#include <QRectF>
#include <QList>

class Carlos;
class Soldado;

// Clase abstracta con lo comun a todos los niveles.
// NivelSelva (fase 2) y NivelRio (fase 5) heredan de aqui.
// Las animas se agregan en la fase 4.
class Nivel
{
public:
    Nivel();
    virtual ~Nivel();

    virtual void cargar() = 0;
    virtual void actualizar(double dt) = 0;

    virtual bool haGanado() const;
    virtual bool haPerdido() const;
    virtual void reiniciar();

    Carlos *getCarlos() const { return carlos; }
    const QList<Soldado *> &getSoldados() const { return soldados; }
    QRectF getMeta() const { return meta; }
    QRectF getLimites() const { return limites; }

protected:
    void liberar();   // borra todas las entidades creadas con new

    Carlos *carlos;
    QList<Soldado *> soldados;   // memoria dinamica: el nivel es el dueno
    QRectF meta;
    QRectF limites;
    bool capturado;
};

#endif
