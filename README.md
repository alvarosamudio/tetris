# Tetris

Implementacion nativa del clasico **Tetris** desarrollada con **Qt6** y **Deepin Tool Kit (DTK6)** para Deepin Linux.

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
./tetris
```

O si se instalo globalmente:

```bash
sudo make install
tetris
```

## Controles

| Tecla | Accion |
|-------|--------|
| `Flecha izquierda` / `Flecha derecha` | Mover pieza |
| `Flecha arriba` | Rotar pieza |
| `Flecha abajo` | Caida suave |
| `Espacio` | Caida instantanea (Hard Drop) |
| `P` | Pausar / Reanudar |
