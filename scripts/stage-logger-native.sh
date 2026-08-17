#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
swift_root="$(cd "${script_dir}/.." && pwd)"
connector_root="$(cd "${swift_root}/../.." && pwd)"
core_root="${COAKKA_CORE_NATIVE_ROOT:-${connector_root}/../coakkaCoreNativeDev}"

native_core_version="1.2.1"
native_git_commit="f50756ebff0d"
native_package_version="${native_core_version}+${native_git_commit}"
staging_root="${COAKKA_LOGGER_STAGING_ROOT:-}"
source_native="${staging_root}/macos-aarch64/libcoakka_logger_core.dylib"
resource_root="${swift_root}/Sources/CoAkkaLogger/Resources/macos-aarch64"
header_root="${swift_root}/Sources/CoAkkaLoggerC/include/coakka/logger"
target_native="${resource_root}/libcoakka_logger_core.10.dylib"
target_core_header="${header_root}/core.h"
target_utils_header="${header_root}/utils.h"

if [[ -z "${staging_root}" ]]; then
  if bash "${script_dir}/verify-native-payload.sh" >/dev/null 2>&1 &&
      [[ -f "${target_core_header}" && -f "${target_utils_header}" ]]; then
    echo "[swift-stage-logger] using exact bundled macOS 13 logger native ${native_package_version}"
    exit 0
  fi
  echo "[swift-stage-logger] set COAKKA_LOGGER_STAGING_ROOT to refresh the native resource" >&2
  exit 1
fi

if [[ ! -f "${source_native}" || ! -f "${core_root}/logger/include/coakka/logger/core.h" ]]; then
  echo "[swift-stage-logger] staged native/header inputs not found" >&2
  echo "[swift-stage-logger] expected source native: ${source_native}" >&2
  echo "[swift-stage-logger] expected core headers under: ${core_root}/logger/include/coakka/logger" >&2
  exit 1
else
  mkdir -p "${resource_root}" "${header_root}"
  install -m 0755 "${source_native}" "${target_native}"
  install -m 0644 "${core_root}/logger/include/coakka/logger/core.h" "${target_core_header}"
  install -m 0644 "${core_root}/logger/include/coakka/logger/utils.h" "${target_utils_header}"

  echo "[swift-stage-logger] staged macOS ARM64 logger native ${native_package_version}"
fi

native_count="$(find "${swift_root}/Sources/CoAkkaLogger/Resources" -type f -name '*.dylib' | wc -l | tr -d ' ')"
if [[ "${native_count}" != "1" ]]; then
  echo "[swift-stage-logger] expected exactly one bundled macOS dylib, found ${native_count}" >&2
  find "${swift_root}/Sources/CoAkkaLogger/Resources" -type f -name '*.dylib' >&2
  exit 1
fi
