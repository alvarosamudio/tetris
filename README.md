# Dropix

Juego de bloques para Deepin Linux desarrollado con **Qt6** y **Deepin Tool Kit (DTK6)**.

## Caracteristicas

- Motor nativo en C++ con Qt para rendimiento optimo
- Integracion DTK con `DMainWindow`, `DTitlebar` y widgets nativos
- Compatible con modo oscuro y claro de Deepin
- Controles fluidos con respuesta inmediata

## Dependencias

```bash
sudo apt install qt6-base-dev qt6-multimedia-dev libdtk6widget-dev cmake g++
```

## Compilar

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Ejecutar

```bash
./dropix
```

O si se instalo globalmente:

```bash
sudo make install
dropix
```

## Controles

| Tecla | Accion |
|-------|--------|
| `Flecha izquierda` / `Flecha derecha` | Mover pieza |
| `Flecha arriba` | Rotar pieza |
| `Flecha abajo` | Caida suave |
| `Espacio` | Caida instantanea (Hard Drop) |
| `P` | Pausar / Reanudar |

## Aviso legal / Legal notice

"Tetris" es una marca registrada de Tetris Holding, LLC. Este proyecto es una
implementacion independiente, escrita desde cero y publicada bajo licencia GPL-3.0;
**no esta afiliado, patrocinado ni aprobado por The Tetris Company o Tetris Holding, LLC**.

"Tetris" is a registered trademark of Tetris Holding, LLC. This project is an
independent, from-scratch implementation released under the GPL-3.0 license; it is
**not affiliated with, sponsored by, or endorsed by The Tetris Company or Tetris
Holding, LLC**.
