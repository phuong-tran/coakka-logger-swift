# CoAkka Logger Swift Connector

<p align="center">
  <img src="https://raw.githubusercontent.com/phuong-tran/coakka-samples/main/docs/assets/brand/coakka-logo.png" alt="CoAkka" width="480">
</p>

[![CI](https://github.com/phuong-tran/coakka-logger-swift/actions/workflows/swift-ci.yml/badge.svg)](https://github.com/phuong-tran/coakka-logger-swift/actions/workflows/swift-ci.yml)
[![Version](https://img.shields.io/badge/version-v1.2.2-blue)](https://github.com/phuong-tran/coakka-logger-swift/releases/tag/v1.2.2)
[![License: file-scoped](https://img.shields.io/badge/license-file--scoped-blue)](PACKAGE-LICENSE.md)
[![Funding](https://img.shields.io/badge/funding-Ko--fi-ff5f5f)](https://ko-fi.com/phuongnamtran)

SwiftPM connector package for the standalone CoAkka native logger core.

The current Swift logger package supports macOS ARM64. Use the compatibility
matrix for exact package and native-platform evidence.

SwiftPM package URL:

```text
https://github.com/phuong-tran/coakka-logger-swift.git
```

Use the exact version `1.2.2`.

Public package links:

| Link | Purpose |
| --- | --- |
| [GitHub Release v1.2.2](https://github.com/phuong-tran/coakka-logger-swift/releases/tag/v1.2.2) | SwiftPM package release with the macOS ARM64 native logger. |
| [Logger sample](https://github.com/phuong-tran/coakka-samples/tree/main/logger/swift/basic) | Runnable bounded logger sample. |
| [Compatibility matrix](https://github.com/phuong-tran/coakka-publish/blob/main/docs/compatibility-matrix.md) | Current native generation and package-manager status. |

## New To CoAkka Logger

CoAkka Logger gives a host application a small language-native logging API
while the native core owns queueing, pressure behavior, drain semantics, sink
behavior, and platform library loading.

Use these public repositories to orient first:

| Repository | Use it for | Link |
| --- | --- | --- |
| `coakka-samples` | Runnable examples and code you can inspect first. | https://github.com/phuong-tran/coakka-samples |
| `coakka-publish` | Released packages, native archives, manifests, checksums, compatibility matrix, and release notes. | https://github.com/phuong-tran/coakka-publish |

Read the deeper package docs:

- [Why CoAkka Logger matters](docs/coakka-logger.md)
- [CoAkka ecosystem map](docs/coakka-ecosystem.md)

## First Run From Source

```sh
bash scripts/smoke-package.sh
```

The smoke stages the macOS ARM64 native logger library, runs `swift test`, and
executes the package smoke binary.

Run a clean SwiftPM consumer smoke:

```sh
bash scripts/smoke-consumer.sh
```

Export the package into a public SwiftPM repository checkout:

```sh
bash scripts/export-module-repo.sh ../coakka-logger-swift
```

## API Shape

```swift
import CoAkkaLogger

let logger = try Logger.start(
    spec: LoggerSpec(systemName: "swift-first-run", minLevel: .info)
)
defer {
    try? logger.close()
}

let sequence = try logger.info("orders", #"{"event":"accepted"}"#)
let record = try logger.awaitNext(timeoutMs: 1_000)
print(sequence ?? 0, record?.category ?? "", record?.message ?? "")
```

The Swift API is intentionally small:

- `Logger.start(...)` owns native logger lifecycle
- `info`, `warn`, `error`, `debug`, `trace`, and `fatal` submit records
- `awaitNext(...)` drains the manual-drain path used by tests and samples
- `stats()` exposes the bounded queue counters needed for smoke and diagnostics

## Native Boundary

The package wraps the public logger C ABI and bundles one macOS ARM64 native
logger library rebuilt from exact native snapshot
`f50756ebff0d2635243093baf533993c4d9fe9dc`. The logical native generation
remains `1.2.1+f50756ebff0d`; package `1.2.2` corrects the dylib deployment
target to macOS `13.0` without changing the public logger ABI.

The bundled resource is:

```text
Sources/CoAkkaLogger/Resources/macos-aarch64/libcoakka_logger_core.10.dylib
```

There should be no Linux `.so` files or Windows `.dll` files in this logger
package. The separately versioned
[`coakka-runtime-swift`](https://github.com/phuong-tran/coakka-runtime-swift)
package is the current Swift runtime lane and carries five runtime payloads;
this logger package remains intentionally macOS ARM64.

Verify that payload shape directly:

```sh
bash scripts/verify-native-payload.sh
```

The verifier pins SHA-256
`28db676f3a56ea67c909697d82453edd76372bd8c9916dd43250f453cfedc87e`
and rejects a Mach-O minimum OS version other than `13.0`.


## License

**Free for application use, including commercial and production use.**

Connector source, generated bindings, type declarations, examples, and package
documentation use the [Apache License, Version 2.0](https://github.com/phuong-tran/coakka-samples/blob/main/LICENSE).
Bundled native files use the [CoAkka Native Artifact License 1.2](https://github.com/phuong-tran/coakka-samples/blob/main/NATIVE-LICENSE.md).
Those native terms permit ordinary application and SaaS use but require a
separate agreement to sell or offer CoAkka itself as managed runtime or
infrastructure.

See [CoAkka Package Licensing](https://github.com/phuong-tran/coakka-samples/blob/main/docs/package-licensing.md)
for the file-scope map. This repository also carries offline `LICENSE`,
`NATIVE-LICENSE.md`, `PACKAGE-LICENSE.md`, and `NOTICE` copies.
