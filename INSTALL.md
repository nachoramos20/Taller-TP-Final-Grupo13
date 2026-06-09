# Instrucciones de instalación

## Instalación automática

Ejecutar el script instalador:

```bash
./install.sh
```

Este script descarga todas las dependencias y compila el proyecto automaticamente.

---

## Instalación manual 

En caso de que ./install.sh no funcione


### 1. Instalar dependencias del sistema

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git pkg-config \
    libopus-dev libopusfile-dev libxmp-dev \
    libfluidsynth-dev fluidsynth \
    libwavpack1 libwavpack-dev wavpack \
    libfreetype-dev \
    libasound2-dev libpulse-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
    libxi-dev libxinerama-dev libxss-dev \
    libgl1-mesa-dev libgles2-mesa-dev libegl1-mesa-dev \
    libsdl2-ttf-dev
```

### 2. Limpiar builds viejas

```bash
make clean
```


### 3. Compilar el proyecto

```bash
make compile-debug
```

Esto generará los ejecutables en el directorio `build/`:

| Ejecutable | Descripción |
|---|---|
| `build/argentum_grupo13_client` | Cliente del juego |
| `build/argentum_grupo13_server` | Servidor del juego |
| `build/argentum_grupo13_editor` | Editor de mapas |
| `build/argentum_grupo13_tests` | Tests unitarios |