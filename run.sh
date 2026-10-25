#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
mkdir -p build
language="${BW_LANG:-en}"
args=("$@")
for ((i=0; i<${#args[@]}; i++)); do
  if [[ "${args[i]}" == --lang && $((i+1)) -lt ${#args[@]} ]]; then language="${args[i+1]}"; fi
done
compiler="${CC:-cc}"
if ! command -v "$compiler" >/dev/null 2>&1; then
  if [[ -x .toolchain/root/usr/bin/x86_64-linux-gnu-gcc-15 ]]; then
    compiler="$PWD/.toolchain/root/usr/bin/x86_64-linux-gnu-gcc-15"
  else
    if [[ "$language" == pt-BR ]]; then
      echo 'Compilador C não encontrado. Instale: sudo apt install build-essential' >&2
    else
      echo 'C compiler not found. Install: sudo apt install build-essential' >&2
    fi
    exit 1
  fi
fi
"$compiler" -std=c11 -O2 -Wall -Wextra -Wpedantic -pthread src/main.c src/i18n.c src/mapping.c src/platform_linux.c -lm -o build/boreal-wheel
if [[ "${1:-}" == --build-only ]]; then exit 0; fi
port=4580
args=("$@")
for ((i=0; i<${#args[@]}; i++)); do
  if [[ "${args[i]}" == --port && $((i+1)) -lt ${#args[@]} ]]; then port="${args[i+1]}"; fi
done
if [[ "${BW_NO_BROWSER:-0}" != 1 ]] && command -v xdg-open >/dev/null; then
  (sleep 1; xdg-open "http://127.0.0.1:$port" >/dev/null 2>&1 || true) &
fi
exec ./build/boreal-wheel "${args[@]}"
