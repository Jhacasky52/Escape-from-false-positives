#ifndef RASGO_H
#define RASGO_H

#include <QString>

class Carlos;
class Nivel;

// Caracteristica del protagonista que cambia su forma de jugar.
// En cada actualizacion Carlos reinicia sus modificadores y cada rasgo los ajusta.
class Rasgo
{
public:
    explicit Rasgo(const QString &nombre);
    virtual ~Rasgo() = default;

    virtual void aplicar(Carlos *carlos, Nivel *nivel, double dt) = 0;

    QString getNombre() const { return nombre; }

protected:
    QString nombre;
};

#endif
