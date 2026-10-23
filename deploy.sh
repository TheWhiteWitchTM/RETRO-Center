#!/bin/bash
# Build RETRO Center, then write dist/ so it runs with or without Qt installed.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
"$ROOT/build.sh"
"$ROOT/scripts/bundle-dist.sh"
