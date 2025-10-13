# !/usr/bin/env bash
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/_shared.sh"
info "removing local installation..."

# Local installation paths
INSTALL_BIN="$HOME/.local/bin"
INSTALL_LIB="$HOME/.local/lib"
INSTALL_SHARE=$HOME/.local/share/via

rm -rf "$INSTALL_SHARE"
rm -f "$INSTALL_BIN/via" "$INSTALL_LIB/libvia-core.so"

info "uninstallation complete!"
