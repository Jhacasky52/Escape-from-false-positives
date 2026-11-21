# Los que no volvieron

Videojuego de terror en C++ y Qt 6 basado en los "falsos positivos" en el Catatumbo (2002-2008).
Carlos, un campesino que fue testigo, debe huir de la patrulla y llegar a Cúcuta para denunciar,
guiado por las ánimas de las víctimas.

## Cómo abrirlo

1. En Qt Creator: **Archivo > Abrir archivo o proyecto** y elegir `CMakeLists.txt`.
2. Seleccionar el kit de Qt 6 (por ejemplo *Desktop Qt 6.x MinGW 64-bit*) y pulsar **Configurar proyecto**.
3. Ejecutar con el botón verde (objetivo `LosQueNoVolvieron`).
4. Para correr las pruebas de las físicas, cambiar el objetivo a `pruebas_fisica` (abajo a la izquierda, icono del monitor) y ejecutar.

## Estado actual: fases 0 y 1

La ventana muestra una **escena de prueba** con las cuatro físicas funcionando:

| Física | Dónde se ve | Ecuación |
| --- | --- | --- |
| Tiro parabólico | Piedra que lanza Carlos (mantener Espacio y soltar) | x = v0·cos(θ)·t, z = v0·sin(θ)·t − ½·g·t² |
| Péndulo | Cono de la linterna del soldado | θ(t) = θ0·cos(√(g/L)·t) |
| Fluido con arrastre | Carlos arrastrado por el río (mantener J para nadar) | m·dv/dt = F + k·(v_río − v) − b·v |
| Oscilación armónica | Tronco en el oleaje y vaivén de las ánimas | y(t) = A·sin(ω·t + φ) |

Controles: WASD o flechas para mover, Shift para correr, Espacio para lanzar, J para nadar, R para reiniciar el río.

## Estructura

```
logica/            capa lógica (las clases del diagrama del Momento II, sin gráficos)
  control/         Juego, EstadoJuego
  niveles/         Nivel (abstracta)
  entidades/       Entidad, Personaje, Carlos
  fisica/          ModeloFisico y los 4 movimientos
vista/             capa gráfica de Qt
  EscenaJuego      bucle de juego con paso fijo (1/60 s)
  EscenaPruebaFisica  escena de prueba de la fase 1
  VentanaPrincipal
pruebas/           pruebas automáticas de las físicas
recursos/sprites/  sprites (se compilan dentro del ejecutable como :/sprites/...)
```

La lógica no depende de los gráficos: cada entidad lógica guarda su posición y estado,
y la escena solo lee esos datos en cada cuadro para mover los sprites.

## Ruta

- [x] Fase 0: proyecto, ventana, bucle de juego
- [x] Fase 1: clases base y las 4 físicas
- [ ] Fase 2: nivel 1 jugable (patrullas, linternas, captura, meta)
- [ ] Fase 3: piedras con ruido, escondites, pánico y latidos
- [ ] Fase 4: ánimas (percepción, razonamiento, acción, aprendizaje)
- [ ] Fase 5: nivel 2, el río
- [ ] Fase 6: menús, guardar partida, final con Ramón, sonido
