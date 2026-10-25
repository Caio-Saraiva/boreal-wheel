#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
./run.sh --build-only
compiler="${CC:-cc}"
if ! command -v "$compiler" >/dev/null 2>&1; then compiler="$PWD/.toolchain/root/usr/bin/x86_64-linux-gnu-gcc-15"; fi
"$compiler" -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -Isrc tests/test_mapping.c src/mapping.c -lm -o build/test-mapping
./build/test-mapping
python3 tests/test_api.py
