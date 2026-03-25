#!/bin/bash
# Thin wrapper for package-macos.sh
# Usage: ./package-macos.sh [debug|release]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${SCRIPT_DIR}/scripts/build/package-macos.sh" "$@"
