# !/usr/bin/env bash
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/_shared.sh"
info "running local installation..."

# Local installation paths
INSTALL_BIN="$HOME/.local/bin"
INSTALL_LIB="$HOME/.local/lib"
INSTALL_SHARE=$HOME/.local/share/via
BUILD_DIR=build/src

# Check required binaries
function check() {
    info "checking for $1..."
    local name="${1^^}_PATH"
    local binary
    if binary=$(which "$1" 2>/dev/null); then
        info "found $1!"
    else
        error "could not find $1"
        info "please install $1 before running the installation script!"
        exit 1
    fi
}

info "checking for required binaries..."
check "ninja"
check "vcpkg"
check "cmake"
check "g++"

if [ -z "$VCPKG_ROOT" ]; then
    error "VCPKG_ROOT not defined"
    info "please follow the official installation instructions for vcpkg at https://vcpkg.io/en/"
    exit 1
fi

# Build everything
info "building via..."
cmake -G Ninja -B build -D CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j=$(nproc 2>/dev/null || echo 1)

# Copy executable
info "installing executable..."
cp "$BUILD_DIR/via-cli/via-cli" "$INSTALL_BIN/via"
mkdir -p "$INSTALL_BIN"

if [[ ":$PATH:" != *":$INSTALL_BIN:"* ]]; then
    info "adding $INSTALL_BIN to PATH..."
    echo "export PATH=\"$INSTALL_BIN:$PATH\"" >> "$HOME/.bashrc"
fi

# Add library
info "insatlling shared library..."
mkdir -p "$INSTALL_LIB"
cp "$BUILD_DIR/via-core/libvia-core.so" "$INSTALL_LIB/via-core.so"

if [[ ":$LD_LIBRARY_PATH:" != *":$INSTALL_LIB:"* ]]; then
    info "adding $INSTALL_LIB to LD_LIBRARY_PATH..."
    export LD_LIBRARY_PATH="$INSTALL_LIB:${LD_LIBRARY_PATH:-}"
    if ! grep -Fxq "export LD_LIBRARY_PATH=\"$INSTALL_LIB:\$LD_LIBRARY_PATH\"" "$HOME/.bashrc"; then
        echo "export LD_LIBRARY_PATH=\"$INSTALL_LIB:\$LD_LIBRARY_PATH\"" >> "$HOME/.bashrc"
    fi
fi

# Initialize language core directories
info "installing language core libraries..."
mkdir -p "$INSTALL_SHARE"
mkdir -p "$INSTALL_SHARE/lib"
shopt -s nullglob
for dir in $BUILD_DIR/via-lib/lib/*/; do
    name=$(basename "$dir")
    info "installing library '$name'..."
    cp -R "$dir" "$INSTALL_SHARE/lib/$name"
done
shopt -u nullglob

info "installation complete!"
exit 0
