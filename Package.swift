// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "coakka-logger-swift",
    platforms: [
        .macOS(.v13),
    ],
    products: [
        .library(name: "CoAkkaLogger", targets: ["CoAkkaLogger"]),
    ],
    targets: [
        .target(
            name: "CoAkkaLoggerC",
            path: "Sources/CoAkkaLoggerC",
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("include"),
            ]
        ),
        .target(
            name: "CoAkkaLogger",
            dependencies: ["CoAkkaLoggerC"],
            path: "Sources/CoAkkaLogger",
            resources: [
                .copy("Resources"),
            ]
        ),
        .executableTarget(
            name: "CoAkkaLoggerSmoke",
            dependencies: ["CoAkkaLogger"],
            path: "Sources/CoAkkaLoggerSmoke"
        ),
        .testTarget(
            name: "CoAkkaLoggerTests",
            dependencies: ["CoAkkaLogger"],
            path: "Tests/CoAkkaLoggerTests"
        ),
    ]
)
