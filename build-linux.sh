#!/bin/bash
# Thin wrapper for build-linux.sh
# Usage: ./build-linux.sh [debug|release]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "${SCRIPT_DIR}/scripts/build-linux.sh" "$@"
