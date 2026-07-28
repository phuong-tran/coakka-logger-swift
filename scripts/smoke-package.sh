#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
swift_root="$(cd "${script_dir}/.." && pwd)"

bash "${script_dir}/stage-logger-native.sh"
bash "${script_dir}/verify-native-payload.sh"

swift --version
(
  cd "${swift_root}"
  swift test
  swift run CoAkkaLoggerSmoke
)

echo "[swift-logger-smoke] ok"
