import CoAkkaLogger

let info = try Logger.readInfo()
let logger = try Logger.start(
    spec: LoggerSpec(systemName: "swift-logger-smoke", minLevel: .info)
)
defer {
    try? logger.close()
}

let sequence = try logger.info(
    "samples.logger.swift.basic",
    #"{"event":"hello","language":"swift"}"#
)
guard let sequence else {
    throw CoAkkaLoggerError.nativeStatus(
        operation: "logger_core_log",
        status: 3,
        statusName: "QUEUE_FULL"
    )
}

guard let record = try logger.awaitNext(timeoutMs: 1_000) else {
    throw CoAkkaLoggerError.nativeStatus(
        operation: "logger_core_read_next",
        status: 5,
        statusName: "TIMED_OUT"
    )
}

let stats = try logger.stats()
print("coakka_logger_info abi=\(info.abiVersion) version=\(info.runtimeVersion) git=\(info.gitCommit)")
print("coakka_logger_record sequence=\(sequence) level=\(record.levelName) category=\(record.category) message=\(record.message)")
print("coakka_logger_stats emitted=\(stats.emittedCount) delivered=\(stats.deliveredCount) dropped=\(stats.droppedCount)")
