#!/bin/sh
set -eu
cd "$(dirname "$0")"
repo_dir=$(pwd -W 2>/dev/null || pwd)
MSYS_NO_PATHCONV=1 docker run --rm -v "$repo_dir:/src:ro" -w /src \
  -e TEST_USE_SYSTEM_MALLOC="${TEST_USE_SYSTEM_MALLOC:-0}" \
  --entrypoint /bin/sh abseil-dev:latest \
  /src/test/run.sh "$@"
