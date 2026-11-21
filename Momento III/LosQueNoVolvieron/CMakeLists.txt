cmake_minimum_required(VERSION 3.16)

project(LosQueNoVolvieron VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

# Todas las imagenes de recursos/sprites entran al ejecutable como :/sprites/...
file(GLOB_RECURSE SPRITES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/recursos/sprites/*.png)

qt_add_executable(LosQueNoVolvieron
    main.cpp

    # ---- Capa logica ----
    logica/control/Juego.h            logica/control/Juego.cpp
    logica/niveles/Nivel.h            logica/niveles/Nivel.cpp

    logica/entidades/Entidad.h        logica/entidades/Entidad.cpp
    logica/entidades/Personaje.h      logica/entidades/Personaje.cpp
    logica/entidades/Carlos.h         logica/entidades/Carlos.cpp

    logica/fisica/ModeloFisico.h          logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoParabolico.h  logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoPendular.h    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoFluido.h      logica/fisica/MovimientoFluido.cpp
    logica/fisica/MovimientoOscilatorio.h logica/fisica/MovimientoOscilatorio.cpp

    # ---- Capa grafica (Qt) ----
    vista/VentanaPrincipal.h  vista/VentanaPrincipal.cpp
    vista/EscenaJuego.h       vista/EscenaJuego.cpp
    vista/EscenaPruebaFisica.h vista/EscenaPruebaFisica.cpp
)

qt_add_resources(LosQueNoVolvieron "recursos"
    PREFIX "/"
    BASE recursos
    FILES ${SPRITES}
)

target_include_directories(LosQueNoVolvieron PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/logica
)

target_link_libraries(LosQueNoVolvieron PRIVATE Qt6::Widgets)

set_target_properties(LosQueNoVolvieron PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

# ---- Pruebas de las fisicas (consola, sin graficos) ----
qt_add_executable(pruebas_fisica
    pruebas/pruebas_fisica.cpp
    logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoFluido.cpp
    logica/fisica/MovimientoOscilatorio.cpp
)
target_include_directories(pruebas_fisica PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logica)
target_link_libraries(pruebas_fisica PRIVATE Qt6::Core)
set_target_properties(pruebas_fisica PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
