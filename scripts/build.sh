#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"

BUILD_TYPE="${BUILD_TYPE:-Debug}"

RESET="\033[0m"
BOLD="\033[1m"
GREEN="\033[32m"
RED="\033[31m"
CYAN="\033[36m"

run_step()
{
    local message="$1"
    shift

    printf "\r\033[K${CYAN}%s${RESET}" "$message"

    local output
    if ! output=$("$@" 2>&1); then
        printf "\r\033[K${RED}✗ %s${RESET}\n" "$message"
        printf '%s\n' "$output"
        exit 1
    fi

    printf "\r\033[K${GREEN}✓ %s${RESET}\n" "$message"
}

run_step "[1/3] Configuring CMake..." \
    cmake -S "$ROOT_DIR" \
          -B "$BUILD_DIR" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [[ ! -e "$ROOT_DIR/compile_commands.json" &&
      ! -L "$ROOT_DIR/compile_commands.json" ]]; then
    ln -s "build/compile_commands.json" \
          "$ROOT_DIR/compile_commands.json"
fi

run_step "[2/3] Building Miyusic..." \
    cmake --build "$BUILD_DIR" --parallel

printf "\r\033[K${GREEN}${BOLD}✓ [3/3] Miyusic build finished!${RESET}\n"
printf "    Output: ${BOLD}%s${RESET}\n" "$BUILD_DIR/miyusic"
