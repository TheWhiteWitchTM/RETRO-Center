#!/bin/bash
# Build RETRO Center against Qt 6. Uses the Qt in ~/Qt/6.12.0 when it is present.
set -euo pipefail
root="$(cd "$(dirname "$0")" && pwd)"
qmake_bin="${QMAKE:-}"
if [[ -z "$qmake_bin" ]]; then
  if [[ -x "${HOME}/Qt/6.12.0/gcc_64/bin/qmake" ]]; then
    qmake_bin="${HOME}/Qt/6.12.0/gcc_64/bin/qmake"
  elif command -v qmake6 >/dev/null 2>&1; then
    qmake_bin="$(command -v qmake6)"
  else
    qmake_bin="$(command -v qmake)"
  fi
fi
python3 "$root/tools/make_demos.py"
mkdir -p "$root/build"
cd "$root/build"
"$qmake_bin" "$root/retrocenter.pro" -o Makefile
make -j"$(nproc)"
echo "Built $root/build/retrocenter"
