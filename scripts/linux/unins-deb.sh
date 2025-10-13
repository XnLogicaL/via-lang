#!/usr/bin/env bash
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/_shared.sh"
info "removing system-wide installation..."

# System-wide installation paths
INSTALL_BIN="/usr/local/bin"
INSTALL_LIB="/usr/local/lib"
INSTALL_SHARE="/usr/local/share/via"

rm -rf "$INSTALL_SHARE"
rm -f "$INSTALL_BIN/via" "$INSTALL_LIB/libvia-core.so"

info "uninstallation complete!"
