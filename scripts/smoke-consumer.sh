#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
swift_root="$(cd "${script_dir}/.." && pwd)"
work_root="$(mktemp -d "${TMPDIR:-/tmp}/coakka-logger-swift-consumer.XXXXXX")"

bash "${script_dir}/stage-logger-native.sh"
bash "${script_dir}/verify-native-payload.sh"

mkdir -p "${work_root}/Sources/ConsumerSmoke"
cat >"${work_root}/Package.swift" <<EOF
// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "CoAkkaLoggerSwiftConsumerSmoke",
    platforms: [.macOS(.v13)],
    dependencies: [
        .package(name: "coakka-logger-swift", path: "${swift_root}"),
    ],
    targets: [
        .executableTarget(
            name: "ConsumerSmoke",
            dependencies: [
                .product(name: "CoAkkaLogger", package: "coakka-logger-swift"),
            ]
        ),
    ]
)
EOF

cat >"${work_root}/Sources/ConsumerSmoke/main.swift" <<'EOF'
import CoAkkaLogger

let logger = try Logger.start(spec: LoggerSpec(systemName: "swift-consumer-smoke", minLevel: .info))
defer {
    try? logger.close()
}

let sequence = try logger.info("consumer.swift", #"{"ok":true}"#)
let record = try logger.awaitNext(timeoutMs: 1_000)
print("consumer_sequence=\(sequence ?? 0) category=\(record?.category ?? "") message=\(record?.message ?? "")")
EOF

(
  cd "${work_root}"
  swift run ConsumerSmoke
)

echo "[swift-logger-consumer-smoke] ok ${work_root}"
