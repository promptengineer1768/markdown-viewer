#!/bin/bash
# Thin wrapper for build-macos.sh
# Usage: ./build-macos.sh [debug|release]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${SCRIPT_DIR}/scripts/build-macos.sh" "$@"
