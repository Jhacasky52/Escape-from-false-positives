cmake_minimum_required(VERSION 3.16)

project(LosQueNoVolvieron VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
# Qt Multimedia es opcional (en el instalador de Qt es un modulo aparte).
# Si esta, se oyen los latidos; si no, el juego funciona igual sin sonido.
find_package(Qt6 QUIET COMPONENTS Multimedia)

qt_standard_project_setup()

# Sprites y mapas entran al ejecutable como :/sprites/... y :/mapas/...
file(GLOB_RECURSE SPRITES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/recursos/sprites/*.png
     ${CMAKE_CURRENT_SOURCE_DIR}/recursos/mapas/*.txt
     ${CMAKE_CURRENT_SOURCE_DIR}/recursos/sonidos/*.wav)

qt_add_executable(LosQueNoVolvieron
    main.cpp

    # ---- Capa logica ----
    logica/control/Juego.h            logica/control/Juego.cpp
    logica/control/Partida.h          logica/control/Partida.cpp
    logica/niveles/Nivel.h            logica/niveles/Nivel.cpp
    logica/niveles/NivelSelva.h       logica/niveles/NivelSelva.cpp
    logica/niveles/NivelRio.h         logica/niveles/NivelRio.cpp

    logica/entidades/Entidad.h        logica/entidades/Entidad.cpp
    logica/entidades/Personaje.h      logica/entidades/Personaje.cpp
    logica/entidades/Carlos.h         logica/entidades/Carlos.cpp
    logica/entidades/Soldado.h        logica/entidades/Soldado.cpp
    logica/entidades/Linterna.h       logica/entidades/Linterna.cpp
    logica/entidades/Piedra.h         logica/entidades/Piedra.cpp
    logica/entidades/Tronco.h         logica/entidades/Tronco.cpp

    logica/rasgos/Rasgo.h             logica/rasgos/Rasgo.cpp
    logica/rasgos/ConocedorDelMonte.h logica/rasgos/ConocedorDelMonte.cpp
    logica/rasgos/Miedoso.h           logica/rasgos/Miedoso.cpp

    logica/agente/Estimulo.h          logica/agente/Decision.h
    logica/agente/Percepcion.h        logica/agente/Percepcion.cpp
    logica/agente/Razonamiento.h      logica/agente/Razonamiento.cpp
    logica/agente/Accion.h            logica/agente/Accion.cpp
    logica/agente/Aprendizaje.h       logica/agente/Aprendizaje.cpp
    logica/agente/Anima.h             logica/agente/Anima.cpp
    logica/agente/AnimaRamon.h        logica/agente/AnimaRamon.cpp

    logica/fisica/ModeloFisico.h          logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoParabolico.h  logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoPendular.h    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoFluido.h      logica/fisica/MovimientoFluido.cpp
    logica/fisica/MovimientoOscilatorio.h logica/fisica/MovimientoOscilatorio.cpp

    # ---- Capa grafica (Qt) ----
    vista/VentanaPrincipal.h  vista/VentanaPrincipal.cpp
    vista/EscenaJuego.h       vista/EscenaJuego.cpp
    vista/EscenaPruebaFisica.h vista/EscenaPruebaFisica.cpp
    vista/EscenaNivelSelva.h  vista/EscenaNivelSelva.cpp
    vista/EscenaNivelRio.h    vista/EscenaNivelRio.cpp
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
if(Qt6Multimedia_FOUND)
    target_link_libraries(LosQueNoVolvieron PRIVATE Qt6::Multimedia)
    target_compile_definitions(LosQueNoVolvieron PRIVATE HAY_MULTIMEDIA)
    message(STATUS "Qt Multimedia encontrado: el juego tendra sonido")
else()
    message(STATUS "Qt Multimedia no esta instalado: el juego no tendra sonido")
endif()

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

# ---- Pruebas de las reglas del nivel 1 (consola, sin graficos) ----
qt_add_executable(pruebas_nivel
    pruebas/pruebas_nivel.cpp
    logica/niveles/Nivel.cpp
    logica/niveles/NivelSelva.cpp
    logica/entidades/Entidad.cpp
    logica/entidades/Personaje.cpp
    logica/entidades/Carlos.cpp
    logica/entidades/Soldado.cpp
    logica/entidades/Linterna.cpp
    logica/entidades/Piedra.cpp
    logica/rasgos/Rasgo.cpp
    logica/rasgos/ConocedorDelMonte.cpp
    logica/rasgos/Miedoso.cpp
    logica/agente/Percepcion.cpp
    logica/agente/Razonamiento.cpp
    logica/agente/Accion.cpp
    logica/agente/Aprendizaje.cpp
    logica/agente/Anima.cpp
    logica/agente/AnimaRamon.cpp
    logica/control/Juego.cpp
    logica/fisica/MovimientoOscilatorio.cpp
    logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoFluido.cpp
)
qt_add_resources(pruebas_nivel "mapas"
    PREFIX "/"
    BASE recursos
    FILES recursos/mapas/selva.txt
)
target_include_directories(pruebas_nivel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logica)
target_link_libraries(pruebas_nivel PRIVATE Qt6::Core)
set_target_properties(pruebas_nivel PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)

# ---- Pruebas del agente inteligente (animas) (consola, sin graficos) ----
qt_add_executable(pruebas_animas
    pruebas/pruebas_animas.cpp
    logica/niveles/Nivel.cpp
    logica/niveles/NivelSelva.cpp
    logica/entidades/Entidad.cpp
    logica/entidades/Personaje.cpp
    logica/entidades/Carlos.cpp
    logica/entidades/Soldado.cpp
    logica/entidades/Linterna.cpp
    logica/entidades/Piedra.cpp
    logica/rasgos/Rasgo.cpp
    logica/rasgos/ConocedorDelMonte.cpp
    logica/rasgos/Miedoso.cpp
    logica/agente/Percepcion.cpp
    logica/agente/Razonamiento.cpp
    logica/agente/Accion.cpp
    logica/agente/Aprendizaje.cpp
    logica/agente/Anima.cpp
    logica/agente/AnimaRamon.cpp
    logica/control/Juego.cpp
    logica/fisica/MovimientoOscilatorio.cpp
    logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoFluido.cpp
)
qt_add_resources(pruebas_animas "mapas_animas"
    PREFIX "/"
    BASE recursos
    FILES recursos/mapas/selva.txt
)
target_include_directories(pruebas_animas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logica)
target_link_libraries(pruebas_animas PRIVATE Qt6::Core)
set_target_properties(pruebas_animas PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)

# ---- Pruebas del nivel 2, el rio (consola, sin graficos) ----
qt_add_executable(pruebas_rio
    pruebas/pruebas_rio.cpp
    logica/niveles/Nivel.cpp
    logica/niveles/NivelSelva.cpp
    logica/niveles/NivelRio.cpp
    logica/entidades/Entidad.cpp
    logica/entidades/Personaje.cpp
    logica/entidades/Carlos.cpp
    logica/entidades/Soldado.cpp
    logica/entidades/Linterna.cpp
    logica/entidades/Piedra.cpp
    logica/entidades/Tronco.cpp
    logica/rasgos/Rasgo.cpp
    logica/rasgos/ConocedorDelMonte.cpp
    logica/rasgos/Miedoso.cpp
    logica/agente/Percepcion.cpp
    logica/agente/Razonamiento.cpp
    logica/agente/Accion.cpp
    logica/agente/Aprendizaje.cpp
    logica/agente/Anima.cpp
    logica/agente/AnimaRamon.cpp
    logica/control/Juego.cpp
    logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoOscilatorio.cpp
    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoFluido.cpp
)
qt_add_resources(pruebas_rio "mapas_rio"
    PREFIX "/"
    BASE recursos
    FILES recursos/mapas/selva.txt
)
target_include_directories(pruebas_rio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logica)
target_link_libraries(pruebas_rio PRIVATE Qt6::Core)
set_target_properties(pruebas_rio PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)

# ---- Pruebas de guardar y cargar la partida (consola, sin graficos) ----
qt_add_executable(pruebas_partida
    pruebas/pruebas_partida.cpp
    logica/control/Partida.cpp
    logica/niveles/Nivel.cpp
    logica/niveles/NivelSelva.cpp
    logica/niveles/NivelRio.cpp
    logica/entidades/Entidad.cpp
    logica/entidades/Personaje.cpp
    logica/entidades/Carlos.cpp
    logica/entidades/Soldado.cpp
    logica/entidades/Linterna.cpp
    logica/entidades/Piedra.cpp
    logica/entidades/Tronco.cpp
    logica/rasgos/Rasgo.cpp
    logica/rasgos/ConocedorDelMonte.cpp
    logica/rasgos/Miedoso.cpp
    logica/agente/Percepcion.cpp
    logica/agente/Razonamiento.cpp
    logica/agente/Accion.cpp
    logica/agente/Aprendizaje.cpp
    logica/agente/Anima.cpp
    logica/agente/AnimaRamon.cpp
    logica/control/Juego.cpp
    logica/fisica/ModeloFisico.cpp
    logica/fisica/MovimientoOscilatorio.cpp
    logica/fisica/MovimientoPendular.cpp
    logica/fisica/MovimientoParabolico.cpp
    logica/fisica/MovimientoFluido.cpp
)
qt_add_resources(pruebas_partida "mapas_partida"
    PREFIX "/"
    BASE recursos
    FILES recursos/mapas/selva.txt
)
target_include_directories(pruebas_partida PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/logica)
target_link_libraries(pruebas_partida PRIVATE Qt6::Core)
set_target_properties(pruebas_partida PROPERTIES WIN32_EXECUTABLE OFF MACOSX_BUNDLE OFF)
