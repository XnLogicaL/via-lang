#!/usr/bin/env bash
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/_shared.sh"
info "running system-wide installation..."

# System-wide installation paths
INSTALL_BIN="/usr/local/bin"
INSTALL_LIB="/usr/local/lib"
INSTALL_SHARE="/usr/local/share/via"
BUILD_DIR="build/src"

# Check required binaries
function check() {
    info "checking for $1..."
    if ! which "$1" >/dev/null 2>&1; then
        error "could not find $1"
        info "please install $1 before running the installation script!"
        exit 1
    else
        info "found $1!"
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
build_type=${BASH_ARGV[0]}
valid_types=("Debug" "Release" "RelWithDebInfo" "MinSizeRel")
if [[ ! " ${valid_types[*]} " =~ " ${build_type} " ]]; then
    error "invalid build type: '${build_type}'"
    info "valid options are: ${valid_types[*]}"
    exit 1
fi

info "building via..."
cmake -G Ninja -B build \
      -D CMAKE_BUILD_TYPE=$build_type \
      -D CMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j=$(nproc 2>/dev/null || echo 1)

# Copy executable
info "installing executable..."
sudo mkdir -p "$INSTALL_BIN"
sudo cp "$BUILD_DIR/via-cli/via-cli" "$INSTALL_BIN/via"
sudo chmod +x "$INSTALL_BIN/via"

# Copy library
info "installing shared library..."
sudo mkdir -p "$INSTALL_LIB"
sudo cp "$BUILD_DIR/via-core/libvia-core.so" "$INSTALL_LIB/via-core.so"

# Update dynamic linker cache
info "updating system linker cache..."
if ! grep -Fxq "$INSTALL_LIB" /etc/ld.so.conf.d/via.conf 2>/dev/null; then
    echo "$INSTALL_LIB" | sudo tee /etc/ld.so.conf.d/via.conf
    sudo ldconfig
fi

# Install language core directories
info "installing language core libraries..."
sudo mkdir -p "$INSTALL_SHARE/lib"
shopt -s nullglob
for dir in $BUILD_DIR/via-lib/lib/*/; do
    name=$(basename "$dir")
    info "installing library '$name'..."
    sudo cp -R "$dir" "$INSTALL_SHARE/lib/$name"
done
shopt -u nullglob

info "installation complete!"
exit 0
