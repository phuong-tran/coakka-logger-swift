# Swift Logger Connector Release Notes

## 1.2.2

- Rebuilt the bundled macOS ARM64 logger dylib from exact native snapshot
  `f50756ebff0d2635243093baf533993c4d9fe9dc` with deployment target `13.0`.
- Pinned the corrected dylib SHA-256 and rejected higher Mach-O minimum OS
  versions during package verification.
- Kept Swift tools `5.9`, the Swift API, logger ABI, and native generation
  `1.2.1+f50756ebff0d` unchanged.
