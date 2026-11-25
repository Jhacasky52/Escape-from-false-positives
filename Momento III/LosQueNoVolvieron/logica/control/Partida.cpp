#include "Partida.h"
#include "Juego.h"
#include "agente/Aprendizaje.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QStandardPaths>

bool Partida::guardar(const Juego &juego, const QString &ruta)
{
    const Aprendizaje *mem = juego.getMemoriaAnimas();

    QJsonArray peligro;
    for (auto it = mem->getMapaPeligro().cbegin(); it != mem->getMapaPeligro().cend(); ++it)
        peligro.append(QJsonArray{it.key().x(), it.key().y(), it.value()});
    QJsonArray capturas;
    for (const QPoint &c : mem->getCapturas())
        capturas.append(QJsonArray{c.x(), c.y()});

    QJsonObject animas;
    animas["obediencia"] = mem->getTasaObediencia();
    animas["capturas"] = capturas;
    animas["peligro"] = peligro;

    QJsonObject raiz;
    raiz["juego"] = "Los que no volvieron";
    raiz["version"] = VERSION;
    raiz["nivel"] = juego.getIndiceNivel();
    raiz["dificultad"] = static_cast<int>(juego.getDificultad().nivel);
    raiz["guardado"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    raiz["animas"] = animas;

    QDir().mkpath(QFileInfo(ruta).absolutePath());
    QFile archivo(ruta);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    archivo.write(QJsonDocument(raiz).toJson(QJsonDocument::Indented));
    return true;
}

bool Partida::cargar(Juego &juego, const QString &ruta)
{
    QFile archivo(ruta);
    if (!archivo.open(QIODevice::ReadOnly))
        return false;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(archivo.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    // Se valida todo antes de tocar el juego
    QJsonObject raiz = doc.object();
    if (raiz["version"].toInt(-1) != VERSION)
        return false;
    int nivel = raiz["nivel"].toInt(-1);
    if (nivel < 0 || nivel >= juego.numeroNiveles())
        return false;
    QJsonObject animas = raiz["animas"].toObject();
    // Las partidas guardadas antes de la dificultad no tienen el campo: Normal
    int dificultad = raiz["dificultad"].toInt(static_cast<int>(NivelDificultad::NORMAL));
    if (dificultad < 0 || dificultad > static_cast<int>(NivelDificultad::DIFICIL))
        return false;

    QMap<QPoint, double> peligro;
    for (const QJsonValue &v : animas["peligro"].toArray()) {
        QJsonArray a = v.toArray();
        if (a.size() != 3)
            return false;
        peligro[QPoint(a[0].toInt(), a[1].toInt())] = a[2].toDouble();
    }
    QVector<QPoint> capturas;
    for (const QJsonValue &v : animas["capturas"].toArray()) {
        QJsonArray a = v.toArray();
        if (a.size() != 2)
            return false;
        capturas.append(QPoint(a[0].toInt(), a[1].toInt()));
    }

    Aprendizaje *mem = juego.getMemoriaAnimas();
    mem->reiniciar();
    mem->setMapaPeligro(peligro);
    mem->setCapturas(capturas);
    mem->setTasaObediencia(animas["obediencia"].toDouble(1.0));
    juego.setDificultad(static_cast<NivelDificultad>(dificultad));
    juego.iniciarEn(nivel);
    return true;
}

bool Partida::existe(const QString &ruta)
{
    return QFile::exists(ruta);
}

bool Partida::borrar(const QString &ruta)
{
    return !QFile::exists(ruta) || QFile::remove(ruta);
}

QString Partida::rutaPorDefecto()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/partida.json";
}
