#!/bin/bash
set -e

echo "corriendo apt-get update"
sudo apt-get update

echo ""
echo "Instalando dependencias"
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

echo ""
echo "Instalacion finalizada"
echo ""
echo "Compilando el proyecto"
make clean
make compile-debug

echo ""
echo "=== Instalación y compilación completadas ==="
echo "Ejecutables generados en: ./build/"
echo "  - argentum_grupo13_client"
echo "  - argentum_grupo13_server"
echo "  - argentum_grupo13_editor"
echo "  - argentum_grupo13_tests"