#ifndef NIVEL_H
#define NIVEL_H

#include <QRectF>

class Carlos;

// Clase abstracta con lo comun a todos los niveles.
// NivelSelva (fase 2) y NivelRio (fase 5) heredan de aqui.
// Los soldados y las animas se agregan en las fases 2 y 4.
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
    QRectF getMeta() const { return meta; }
    QRectF getLimites() const { return limites; }

protected:
    void liberar();   // borra todas las entidades creadas con new

    Carlos *carlos;
    QRectF meta;
    QRectF limites;
    bool capturado;
};

#endif
