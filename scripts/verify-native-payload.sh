#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
swift_root="$(cd "${script_dir}/.." && pwd)"
resource_root="${swift_root}/Sources/CoAkkaLogger/Resources"
native_path="${resource_root}/macos-aarch64/libcoakka_logger_core.10.dylib"
expected_sha256="28db676f3a56ea67c909697d82453edd76372bd8c9916dd43250f453cfedc87e"

if [[ ! -f "${native_path}" ]]; then
  echo "[swift-verify-logger-payload] missing macOS ARM64 native: ${native_path}" >&2
  exit 1
fi

native_entries="$(find "${resource_root}" -type f \( -name '*.so' -o -name '*.dylib' -o -name '*.dll' \) | sort)"
entry_count="$(printf '%s\n' "${native_entries}" | sed '/^$/d' | wc -l | tr -d ' ')"
if [[ "${entry_count}" != "1" ]]; then
  echo "[swift-verify-logger-payload] expected exactly 1 native entry, got ${entry_count}" >&2
  printf '%s\n' "${native_entries}" >&2
  exit 1
fi

if printf '%s\n' "${native_entries}" | grep -Ev '/macos-aarch64/libcoakka_logger_core\.10\.dylib$' >/dev/null; then
  echo "[swift-verify-logger-payload] unexpected native entry found" >&2
  printf '%s\n' "${native_entries}" >&2
  exit 1
fi

if ! file "${native_path}" | grep -q 'Mach-O 64-bit dynamically linked shared library arm64'; then
  echo "[swift-verify-logger-payload] native library is not macOS ARM64 dylib" >&2
  file "${native_path}" >&2
  exit 1
fi

actual_sha256="$(shasum -a 256 "${native_path}" | awk '{print $1}')"
if [[ "${actual_sha256}" != "${expected_sha256}" ]]; then
  echo "[swift-verify-logger-payload] digest mismatch: ${actual_sha256}" >&2
  exit 1
fi

macos_build_version="$(xcrun vtool -show-build "${native_path}")"
macos_minos="$(printf '%s\n' "${macos_build_version}" | awk '$1 == "minos" { print $2 }')"
if [[ "${macos_minos}" != "13.0" ]]; then
  echo "[swift-verify-logger-payload] expected macOS deployment target 13.0, got ${macos_minos:-missing}" >&2
  printf '%s\n' "${macos_build_version}" >&2
  exit 1
fi

echo "[swift-verify-logger-payload] ok macOS deployment target=13.0"
