#!/bin/bash
# Thin wrapper for package-linux.sh
# Usage: ./package-linux.sh [debug|release]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${SCRIPT_DIR}/scripts/build/package-linux.sh" "$@"
