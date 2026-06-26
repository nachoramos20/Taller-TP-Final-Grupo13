#!/bin/bash

set -e

APP_NAME="argentum_grupo13"
REPO_URL="https://github.com/nachoramos20/Taller-TP-Final-Grupo13"

INSTALL_DIR="$HOME/.local/share/$APP_NAME"
BIN_DIR="$HOME/.local/bin"

echo "======================================"
echo " Argentum Online - Grupo 13 Installer "
echo "======================================"

# ── Detectar si estamos dentro del repo ──────────────────────────────────────
REPO_DIR=""

# Caso 1: el script se ejecuta desde dentro del repo (hay un Makefile y CMakeLists.txt)
if [ -f "$(pwd)/Makefile" ] && [ -f "$(pwd)/CMakeLists.txt" ]; then
    REPO_DIR="$(pwd)"
    echo
    echo "Repositorio detectado en: $REPO_DIR"
    echo "Compilando desde el directorio actual..."
else
    # Caso 2: clonar el repo en un directorio temporal
    TMP_DIR="$(mktemp -d)"
    echo
    echo "No se detectó el repositorio. Clonando desde GitHub..."
    git clone "$REPO_URL" "$TMP_DIR"
    REPO_DIR="$TMP_DIR"
    echo "Repositorio clonado en: $REPO_DIR"
fi

cd "$REPO_DIR"

# ── 1. Dependencias ───────────────────────────────────────────────────────────
echo
echo "[1/6] Actualizando repositorios..."
sudo apt-get update

echo
echo "[2/6] Instalando dependencias..."
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libopus-dev \
    libopusfile-dev \
    libxmp-dev \
    libfluidsynth-dev \
    fluidsynth \
    libwavpack1 \
    libwavpack-dev \
    wavpack \
    libfreetype-dev \
    libasound2-dev \
    libpulse-dev \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxss-dev \
    libgl1-mesa-dev \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev

# ── 2. Compilar ───────────────────────────────────────────────────────────────
echo
echo "[3/6] Compilando proyecto..."
make clean
make compile-debug

# ── 3. Tests ──────────────────────────────────────────────────────────────────
echo
echo "[4/6] Ejecutando tests..."
"$REPO_DIR/build/argentum_grupo13_tests"

# ── 4. Instalar ───────────────────────────────────────────────────────────────
echo
echo "[5/6] Instalando archivos..."

mkdir -p "$INSTALL_DIR"
mkdir -p "$BIN_DIR"

rm -rf "$INSTALL_DIR/assets"
rm -rf "$INSTALL_DIR/config"
rm -rf "$INSTALL_DIR/maps"

cp -r "$REPO_DIR/assets"  "$INSTALL_DIR/"
cp -r "$REPO_DIR/config"  "$INSTALL_DIR/"

if [ -d "$REPO_DIR/maps" ]; then
    cp -r "$REPO_DIR/maps" "$INSTALL_DIR/"
fi

cp "$REPO_DIR/build/argentum_grupo13_client"  "$INSTALL_DIR/"
cp "$REPO_DIR/build/server/argentum_grupo13_server" "$INSTALL_DIR/"
cp "$REPO_DIR/build/argentum_grupo13_editor"  "$INSTALL_DIR/"
cp "$REPO_DIR/build/argentum_grupo13_tests"   "$INSTALL_DIR/"

chmod +x "$INSTALL_DIR"/argentum_grupo13_*

# ── 5. Lanzadores ─────────────────────────────────────────────────────────────
echo
echo "Creando lanzadores..."

cat > "$BIN_DIR/argentum_grupo13_client" << EOF
#!/bin/bash
cd "$INSTALL_DIR"
exec ./argentum_grupo13_client "\$@"
EOF

cat > "$BIN_DIR/argentum_grupo13_server" << EOF
#!/bin/bash
cd "$INSTALL_DIR"
exec ./argentum_grupo13_server "\$@"
EOF

cat > "$BIN_DIR/argentum_grupo13_editor" << EOF
#!/bin/bash
cd "$INSTALL_DIR"
exec ./argentum_grupo13_editor "\$@"
EOF

chmod +x "$BIN_DIR/argentum_grupo13_client"
chmod +x "$BIN_DIR/argentum_grupo13_server"
chmod +x "$BIN_DIR/argentum_grupo13_editor"

# ── 6. Limpiar temp si fue clonado ────────────────────────────────────────────
if [ -n "${TMP_DIR:-}" ]; then
    echo
    echo "Limpiando directorio temporal..."
    rm -rf "$TMP_DIR"
fi

# ── Fin ───────────────────────────────────────────────────────────────────────
echo
echo "[6/6] Instalación completada"
echo
echo "======================================"
echo " INSTALACIÓN EXITOSA "
echo "======================================"
echo
echo "Archivos instalados en:"
echo "  $INSTALL_DIR"
echo
echo "Ejecutables disponibles:"
echo
echo "  argentum_grupo13_server"
echo "  argentum_grupo13_client"
echo "  argentum_grupo13_editor"
echo

if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo "ATENCIÓN:"
    echo
    echo "~/.local/bin no está en tu PATH."
    echo
    echo "Agregá esta línea a ~/.bashrc:"
    echo
    echo 'export PATH="$HOME/.local/bin:$PATH"'
    echo
    echo "Luego ejecutá:"
    echo "  source ~/.bashrc"
    echo
fi

echo "Listo."