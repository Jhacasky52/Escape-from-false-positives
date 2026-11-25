#include <QApplication>
#include "vista/VentanaPrincipal.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Nombre de la aplicacion: define la carpeta donde se guarda la partida
    QApplication::setApplicationName("LosQueNoVolvieron");
    QApplication::setOrganizationName("LosQueNoVolvieron");
    VentanaPrincipal ventana;
    ventana.show();
    return app.exec();
}
