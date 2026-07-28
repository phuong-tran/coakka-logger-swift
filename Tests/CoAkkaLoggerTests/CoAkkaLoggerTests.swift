import XCTest
@testable import CoAkkaLogger

final class CoAkkaLoggerTests: XCTestCase {
    func testReadInfoAndLogRecord() throws {
        let info = try Logger.readInfo()
        XCTAssertEqual(info.abiVersion, coakkaLoggerCoreABIVersion)

        let logger = try Logger.start(
            spec: LoggerSpec(systemName: "swift-logger-test", minLevel: .info)
        )
        defer {
            try? logger.close()
        }

        let sequence = try logger.info("tests.logger.swift", "hello from swift")
        XCTAssertNotNil(sequence)

        let record = try logger.awaitNext(timeoutMs: 1_000)
        XCTAssertEqual(record?.level, .info)
        XCTAssertEqual(record?.category, "tests.logger.swift")
        XCTAssertEqual(record?.message, "hello from swift")

        let stats = try logger.stats()
        XCTAssertGreaterThanOrEqual(stats.emittedCount, 1)
        XCTAssertGreaterThanOrEqual(stats.deliveredCount, 1)
    }

    func testMinLevelProbeSkipsBelowThreshold() throws {
        let logger = try Logger.start(
            spec: LoggerSpec(systemName: "swift-logger-level-test", minLevel: .warn)
        )
        defer {
            try? logger.close()
        }

        XCTAssertFalse(logger.isEnabledForCategory("tests.logger.swift", level: .info))
        XCTAssertNil(try logger.info("tests.logger.swift", "below threshold"))
        XCTAssertNil(try logger.poll())
    }
}
