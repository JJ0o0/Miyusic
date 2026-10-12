#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

rm -rf "$ROOT_DIR/build"

printf '\033[32m✓ Miyusic build cleaned!\033[0m\n'