# Los que no volvieron

Videojuego de terror en C++ y Qt 6 basado en los "falsos positivos" en el Catatumbo (2002-2008).
Carlos, un campesino que fue testigo, debe huir de la patrulla y llegar a Cúcuta para denunciar,
guiado por las ánimas de las víctimas.

## Cómo abrirlo

1. En Qt Creator: **Archivo > Abrir archivo o proyecto** y elegir `CMakeLists.txt`.
2. Seleccionar el kit de Qt 6 (por ejemplo *Desktop Qt 6.x MinGW 64-bit*) y pulsar **Configurar proyecto**.
3. Ejecutar con el botón verde (objetivo `LosQueNoVolvieron`).
4. Para correr las pruebas automáticas, cambiar el objetivo a `pruebas_fisica` o `pruebas_nivel` (abajo a la izquierda, icono del monitor) y ejecutar.

## Estado actual: fase 3 (mecánicas completas del nivel 1)

Al abrir el juego empieza el **nivel 1, la selva**: es de noche, la cámara sigue a Carlos y
cuatro soldados patrullan con linternas. Si Carlos queda en la luz unos instantes, lo descubren
y lo persiguen; si lo alcanzan, el nivel se reinicia. Se gana llegando al río (borde derecho).
Los árboles bloquean el paso y la luz. El mapa se edita en `recursos/mapas/selva.txt`.

**Fase 3:**
- **Piedras** (mantener Espacio y soltar): siguen un tiro parabólico; la fuerza depende del tiempo
  presionado. Al caer hacen ruido y los soldados cercanos van a **investigar** (aparece un "?").
  Si chocan con un árbol caen ahí.
- **Soldados**: máquina de estados PATRULLANDO → INVESTIGANDO → PERSIGUIENDO. Para ir a un ruido o
  volver a su ruta usan una búsqueda en anchura (BFS) sobre la cuadrícula que rodea los árboles.
  Los pasos de Carlos también suenan: caminando poco, corriendo mucho.
- **Rasgo Conocedor del monte**: en matorrales y troncos huecos lo ven mucho menos; con E se esconde
  (invisible, máximo 6 s); en la trocha camina un 25 % más rápido.
- **Rasgo Miedoso**: el pánico sube con soldados cerca (más si lo persiguen) y baja quieto o escondido.
  Con pánico alto tiembla (movimiento y puntería imprecisos) y hace más ruido.
- **Latidos**: el corazón del HUD late a 60 + 1,2 × pánico lpm y se oye si Qt Multimedia está instalado
  (es opcional: sin ese módulo el juego compila igual, sin sonido).

Controles: WASD o flechas mover, Shift correr, Espacio (mantener) lanzar piedra, E esconderse,
P pausa, Enter reintentar, F9 escena de prueba de físicas.

### Escena de prueba de físicas (F9)

Muestra las cuatro físicas funcionando:

| Física | Dónde se ve | Ecuación |
| --- | --- | --- |
| Tiro parabólico | Piedra que lanza Carlos (mantener Espacio y soltar) | x = v0·cos(θ)·t, z = v0·sin(θ)·t − ½·g·t² |
| Péndulo | Cono de la linterna del soldado | θ(t) = θ0·cos(√(g/L)·t) |
| Fluido con arrastre | Carlos arrastrado por el río (mantener J para nadar) | m·dv/dt = F + k·(v_río − v) − b·v |
| Oscilación armónica | Tronco en el oleaje y vaivén de las ánimas | y(t) = A·sin(ω·t + φ) |

Controles de la prueba: WASD mover, Shift correr, Espacio lanzar, J nadar, R reiniciar el río.

## Estructura

```
logica/            capa lógica (las clases del diagrama del Momento II, sin gráficos)
  control/         Juego, EstadoJuego
  niveles/         Nivel (abstracta), NivelSelva
  entidades/       Entidad, Personaje, Carlos, Soldado, Linterna, Piedra
  rasgos/          Rasgo, ConocedorDelMonte, Miedoso
  fisica/          ModeloFisico y los 4 movimientos
vista/             capa gráfica de Qt
  EscenaJuego      bucle de juego con paso fijo (1/60 s)
  EscenaNivelSelva    gráficos del nivel 1: mapa, sprites, oscuridad, cámara, HUD
  EscenaPruebaFisica  escena de prueba de la fase 1
  VentanaPrincipal
pruebas/           pruebas automáticas de las físicas y de las reglas del nivel 1
recursos/sprites/  sprites (se compilan dentro del ejecutable como :/sprites/...)
recursos/mapas/    mapas de los niveles en texto
recursos/sonidos/  sonidos (latido)
```

La lógica no depende de los gráficos: cada entidad lógica guarda su posición y estado,
y la escena solo lee esos datos en cada cuadro para mover los sprites.

## Ruta

- [x] Fase 0: proyecto, ventana, bucle de juego
- [x] Fase 1: clases base y las 4 físicas
- [x] Fase 2: nivel 1 jugable (patrullas, linternas, captura, meta)
- [x] Fase 3: piedras con ruido, escondites, pánico y latidos
- [ ] Fase 4: ánimas (percepción, razonamiento, acción, aprendizaje)
- [ ] Fase 5: nivel 2, el río
- [ ] Fase 6: menús, guardar partida, final con Ramón, sonido
