# Los que no volvieron

Videojuego de terror en C++ y Qt 6 basado en los "falsos positivos" en el Catatumbo (2002-2008).
Carlos, un campesino que fue testigo, debe huir de la patrulla y llegar a Cúcuta para denunciar,
guiado por las ánimas de las víctimas.

## Cómo abrirlo

1. En Qt Creator: **Archivo > Abrir archivo o proyecto** y elegir `CMakeLists.txt`.
2. Seleccionar el kit de Qt 6 (por ejemplo *Desktop Qt 6.x MinGW 64-bit*) y pulsar **Configurar proyecto**.
3. Ejecutar con el botón verde (objetivo `LosQueNoVolvieron`).
4. Para correr las pruebas automáticas, cambiar el objetivo a `pruebas_fisica`, `pruebas_nivel`, `pruebas_animas`, `pruebas_rio` o `pruebas_partida` (abajo a la izquierda, icono del monitor) y ejecutar.

## Estado actual: fase 6 (juego completo)

El juego ya tiene todo el recorrido: **menú → selva → río → final en Cúcuta → menú**.

**Fase 6:**
- **Menú principal** (`EscenaMenu`): Nueva partida, Continuar (solo si hay partida guardada), Controles y Salir.
- **Pausa** (P): G guarda la partida, M vuelve al menú. Al perder también se puede volver al menú con M.
- **Guardar partida** (`logica/control/Partida`): un archivo JSON en la carpeta de datos del usuario
  (en Windows `%APPDATA%/LosQueNoVolvieron/LosQueNoVolvieron/partida.json`). Guarda el nivel al que llegó
  Carlos y **lo que aprendieron las ánimas** (mapa de peligro, capturas y obediencia), así que al continuar
  las ánimas siguen evitando las zonas donde lo atraparon. Se guarda solo al pasar al río y con G en la
  pausa; el nivel se retoma desde su comienzo. Si el archivo está dañado, el juego lo avisa y no se rompe.
- **Final** (`EscenaFinal`): Carlos llega a Cúcuta al amanecer, el ánima de Ramón baja a su encuentro
  (`AnimaRamon::reencuentro()`, que también se activa en la lógica al llegar a la otra orilla) y las demás
  ánimas suben al cielo. Cierra con una dedicatoria a las víctimas. Al terminar se borra la partida guardada.
- **Sonido** (`vista/Sonidos`, con Qt Multimedia): ambiente de la selva (grillos y viento) y del río,
  pasos, chapoteo al nadar, retumbo cuando empieza la creciente, golpe al ser atrapado, campana de Ramón
  y los latidos. Los `.wav` se generaron por código y están en `recursos/sonidos/`.

### Niveles

Al elegir Nueva partida empieza el **nivel 1, la selva**: es de noche, la cámara sigue a Carlos y
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

**Fase 4: las ánimas (agente inteligente).** Tres ánimas de víctimas y la de Ramón acompañan a Carlos.
Cada una ejecuta en ciclo cuatro módulos separados (`logica/agente/`):

1. **Percepción** (`Percepcion`): dentro de un radio lee la posición, ruido y pánico de Carlos, los
   soldados, los conos de sus linternas y las celdas vecinas libres, y arma un `Estimulo`.
2. **Razonamiento** (`Razonamiento`): calcula el riesgo de cada celda vecina,
   `riesgo = w1·cercanía de soldados + w2·luz + w3·exposición + w4·peligro aprendido − w5·avance a la meta`,
   y decide por prioridad: OCULTARSE (soldado a menos de 110 px) > ADVERTIR (riesgo alto donde está o
   hacia donde va; señala el refugio más seguro) > CALMAR (pánico > 60) > GUIAR (celda de menor riesgo).
3. **Acción** (`Accion`): se mueve hacia el destino con vaivén (`MovimientoOscilatorio`), cambia de color
   (blanca = seguro, roja parpadeando = peligro, tenue = ocultarse) y, cerca de Carlos, le baja el pánico.
4. **Aprendizaje** (`Aprendizaje`): mapa de peligro por celda (`QMap<QPoint,double>`) con las capturas y
   los avistamientos de soldados, y la tasa de obediencia: si Carlos ignora las indicaciones, las ánimas
   se vuelven más insistentes. Esta memoria la guarda `Juego`, así que **se conserva al reiniciar el nivel**.

Ramón (`AnimaRamon`, luz cálida) solo aparece por momentos o cuando Carlos está aterrorizado.
Con **F1** se ve el razonamiento: riesgo de cada celda, peligro aprendido en rojo y la decisión de cada ánima.

Controles: WASD o flechas mover, Shift correr, Espacio (mantener) lanzar piedra, E esconderse,
P pausa, Enter reintentar, F1 ver el razonamiento de las ánimas, F9 escena de prueba de físicas.

**Fase 5: nivel 2, el río.** Al llegar al río aparece "Presiona Enter para continuar" y empieza el
segundo nivel, visto de lado. Carlos tiene que cruzar río arriba (hacia la derecha) hasta la otra
orilla mientras la corriente lo empuja hacia atrás (`logica/niveles/NivelRio`):

- **Nadar** (A/D): `MovimientoFluido`, m·dv/dt = F + k·(v_río − v) − b·v con m = 1, k = 3, b = 1 y
  F = 480. Sin nadar la corriente lo arrastra; nadando avanza a (480 − 3·70)/4 ≈ 67 px/s.
- **Bucear** (S): la flotación (−9·profundidad) lo devuelve a la superficie; bajo los 30 px las
  linternas no lo ven, pero gasta aire (8 s). En la superficie o sobre una roca respira.
- **Rocas y bancos de arena** (W junto a una para subirse): ahí la corriente no lo mueve y respira,
  pero queda a la vista. Las rocas tapan la luz.
- **Creciente**: cada 45 s el río crece (indicador arriba a la izquierda). El agua sube 50 px, tapa
  las rocas bajas y los bancos (si Carlos está encima, cae al agua) y la corriente pasa de 70 a 150 px/s.
  Solo las rocas altas y las orillas siguen secas.
- **Troncos y escombros** bajan con la corriente y suben y bajan con el oleaje (`MovimientoOscilatorio`,
  clase `Tronco`). Si lo golpean lo empujan río abajo; buceando pasa por debajo.
- **Soldados en la orilla de enfrente**: patrullan con la linterna apuntando al agua (péndulo). Si lo
  ven, lo atrapan. Si hace ruido al nadar se acercan a revisar.
- **Ánimas**: descansan sobre las rocas altas. Además de las reglas de la fase 4, cuando faltan menos
  de 6 s para la creciente y Carlos está en el agua, se ponen rojas y señalan la roca alta más cercana.
- **Se pierde** si lo ven, si se queda sin aire o si la corriente lo arrastra 560 px hacia atrás
  (la franja oscura de la izquierda). Se gana al llegar a la otra orilla.

Controles del río: A/D o flechas nadar, S bucear, W subirse a una roca, P pausa, Enter reintentar, F1 ánimas.

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
  control/         Juego, EstadoJuego, Partida (guardar y cargar en JSON)
  niveles/         Nivel (abstracta), NivelSelva, NivelRio
  entidades/       Entidad, Personaje, Carlos, Soldado, Linterna, Piedra, Tronco
  rasgos/          Rasgo, ConocedorDelMonte, Miedoso
  agente/          Anima, AnimaRamon, Percepcion, Razonamiento, Accion, Aprendizaje, Estimulo, Decision
  fisica/          ModeloFisico y los 4 movimientos
vista/             capa gráfica de Qt
  EscenaJuego      bucle de juego con paso fijo (1/60 s)
  EscenaNivelSelva    gráficos del nivel 1: mapa, sprites, oscuridad, cámara, HUD
  EscenaNivelRio      gráficos del nivel 2: fondo con paralaje, agua, rocas, troncos, aire, creciente
  EscenaMenu          menú principal y controles
  EscenaFinal         final en Cúcuta con Ramón
  EscenaPruebaFisica  escena de prueba de la fase 1
  Sonidos             efectos y ambiente (opcional, con Qt Multimedia)
  VentanaPrincipal    arma el juego y cambia de escena: menú → selva → río → final
pruebas/           pruebas automáticas: físicas, nivel 1, agente (ánimas), nivel 2 (río) y partida
recursos/sprites/  sprites (se compilan dentro del ejecutable como :/sprites/...)
recursos/mapas/    mapas de los niveles en texto
recursos/sonidos/  sonidos (latido, pasos, chapoteo, ambientes, creciente, captura, Ramón)
```

La lógica no depende de los gráficos: cada entidad lógica guarda su posición y estado,
y la escena solo lee esos datos en cada cuadro para mover los sprites.

## Ruta

- [x] Fase 0: proyecto, ventana, bucle de juego
- [x] Fase 1: clases base y las 4 físicas
- [x] Fase 2: nivel 1 jugable (patrullas, linternas, captura, meta)
- [x] Fase 3: piedras con ruido, escondites, pánico y latidos
- [x] Fase 4: ánimas (percepción, razonamiento, acción, aprendizaje)
- [x] Fase 5: nivel 2, el río
- [x] Fase 6: menús, guardar partida, final con Ramón, sonido
