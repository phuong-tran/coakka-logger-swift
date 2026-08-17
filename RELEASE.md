# Releasing The Swift Logger Connector

Current package version is `1.2.2`, paired with native generation
`1.2.1+f50756ebff0d`; publisher signing is absent.

The macOS ARM64 payload is rebuilt from exact native source snapshot
`f50756ebff0d2635243093baf533993c4d9fe9dc` with deployment target `13.0`.
Its SHA-256 is
`28db676f3a56ea67c909697d82453edd76372bd8c9916dd43250f453cfedc87e`.

## Package Verification

```sh
bash scripts/verify-native-payload.sh
swift test
swift run CoAkkaLoggerSmoke
bash scripts/smoke-consumer.sh
```

`xcrun vtool -show-build` must report macOS `minos 13.0`. Running the package
on a newer macOS ARM64 host is not evidence that it was executed on macOS 13.
