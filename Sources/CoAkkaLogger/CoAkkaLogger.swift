import CoAkkaLoggerC
import Foundation

public let coakkaLoggerCoreABIVersion: UInt32 = 10

private let statusOK: Int32 = 0
private let statusQueueFull: Int32 = 3
private let statusTimedOut: Int32 = 5

public enum CoAkkaLoggerError: Error, CustomStringConvertible {
    case unsupportedPlatform(String)
    case nativeLibraryNotFound(String)
    case nativeOpenFailed(String)
    case unexpectedABI(expected: UInt32, actual: UInt32)
    case invalidArgument(String)
    case nativeStatus(operation: String, status: Int32, statusName: String)
    case closed

    public var description: String {
        switch self {
        case let .unsupportedPlatform(message):
            return message
        case let .nativeLibraryNotFound(path):
            return "native logger library not found at \(path)"
        case let .nativeOpenFailed(path):
            return "failed to open native logger library at \(path)"
        case let .unexpectedABI(expected, actual):
            return "unexpected logger ABI version: expected \(expected), got \(actual)"
        case let .invalidArgument(message):
            return "invalid argument: \(message)"
        case let .nativeStatus(operation, status, statusName):
            return "\(operation) failed: \(statusName) (\(status))"
        case .closed:
            return "logger is closed"
        }
    }
}

public enum LoggerLevel: Int32, Sendable {
    case trace = 0
    case debug = 1
    case info = 2
    case warn = 3
    case error = 4
    case fatal = 5
}

public struct LoggerSpec: Sendable {
    public var systemName: String
    public var queueCapacity: UInt32
    public var categoryCapacity: UInt32
    public var messageCapacity: UInt32
    public var minLevel: LoggerLevel

    public init(
        systemName: String = "swiftLogger",
        queueCapacity: UInt32 = 256,
        categoryCapacity: UInt32 = 64,
        messageCapacity: UInt32 = 512,
        minLevel: LoggerLevel = .trace
    ) {
        self.systemName = systemName
        self.queueCapacity = queueCapacity
        self.categoryCapacity = categoryCapacity
        self.messageCapacity = messageCapacity
        self.minLevel = minLevel
    }
}

public struct LoggerInfo: Sendable {
    public let abiVersion: UInt32
    public let runtimeVersion: String
    public let gitCommit: String
    public let docsHint: String
}

public struct LoggerStats: Sendable {
    public let state: Int32
    public let stateName: String
    public let queueCapacity: UInt32
    public let queueDepth: UInt32
    public let queueHighWatermark: UInt32
    public let nextSequence: UInt64
    public let emittedCount: UInt64
    public let deliveredCount: UInt64
    public let droppedCount: UInt64
}

public struct LoggerRecord: Sendable {
    public let sequence: UInt64
    public let wallTimeUnixMs: UInt64
    public let monotonicTimeNs: UInt64
    public let level: LoggerLevel
    public let levelName: String
    public let category: String
    public let message: String
}

public final class Logger {
    private let bindings: NativeBindings
    private let spec: LoggerSpec
    private let lock = NSLock()
    private var handle: OpaquePointer?
    private var stopped = false
    private var closed = false

    public static func start(
        spec: LoggerSpec = LoggerSpec(),
        nativeLibraryPath: String? = nil
    ) throws -> Logger {
        if spec.systemName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty {
            throw CoAkkaLoggerError.invalidArgument("systemName is required")
        }
        if spec.systemName.utf8.contains(0) {
            throw CoAkkaLoggerError.invalidArgument("systemName contains NUL")
        }

        let bindings = try NativeBindings.open(nativeLibraryPath: nativeLibraryPath)
        let abi = coakka_swift_logger_get_abi_version(bindings.pointer)
        if abi != coakkaLoggerCoreABIVersion {
            throw CoAkkaLoggerError.unexpectedABI(expected: coakkaLoggerCoreABIVersion, actual: abi)
        }

        var handle: OpaquePointer?
        let createStatus = spec.systemName.withCString { systemName in
            coakka_swift_logger_create(
                bindings.pointer,
                systemName,
                spec.queueCapacity == 0 ? 256 : spec.queueCapacity,
                spec.categoryCapacity == 0 ? 64 : spec.categoryCapacity,
                spec.messageCapacity == 0 ? 512 : spec.messageCapacity,
                spec.minLevel.rawValue,
                &handle
            )
        }
        try bindings.requireOK(createStatus, operation: "logger_core_create")
        guard let handle else {
            throw CoAkkaLoggerError.nativeStatus(
                operation: "logger_core_create",
                status: -1,
                statusName: "NULL_HANDLE"
            )
        }

        do {
            try bindings.requireOK(
                coakka_swift_logger_start(bindings.pointer, handle),
                operation: "logger_core_start"
            )
        } catch {
            coakka_swift_logger_destroy(bindings.pointer, handle)
            throw error
        }

        return Logger(bindings: bindings, handle: handle, spec: spec.normalized())
    }

    public static func readInfo(nativeLibraryPath: String? = nil) throws -> LoggerInfo {
        let bindings = try NativeBindings.open(nativeLibraryPath: nativeLibraryPath)
        return try bindings.readInfo()
    }

    private init(bindings: NativeBindings, handle: OpaquePointer, spec: LoggerSpec) {
        self.bindings = bindings
        self.handle = handle
        self.spec = spec
    }

    deinit {
        try? close()
    }

    public func isEnabledForCategory(_ category: String, level: LoggerLevel) -> Bool {
        lock.lock()
        defer { lock.unlock() }
        guard !closed, let handle else {
            return false
        }
        return category.withCString { categoryPtr in
            coakka_swift_logger_is_enabled_for_category(
                bindings.pointer,
                handle,
                categoryPtr,
                level.rawValue
            ) != 0
        }
    }

    @discardableResult
    public func log(_ level: LoggerLevel, category: String, message: String) throws -> UInt64? {
        if category.utf8.contains(0) {
            throw CoAkkaLoggerError.invalidArgument("category contains NUL")
        }
        if message.utf8.contains(0) {
            throw CoAkkaLoggerError.invalidArgument("message contains NUL")
        }
        if !isEnabledForCategory(category, level: level) {
            return nil
        }

        lock.lock()
        defer { lock.unlock() }
        guard !closed, let handle else {
            throw CoAkkaLoggerError.closed
        }

        var sequence: UInt64 = 0
        let status = category.withCString { categoryPtr in
            message.withCString { messagePtr in
                coakka_swift_logger_log(
                    bindings.pointer,
                    handle,
                    level.rawValue,
                    categoryPtr,
                    messagePtr,
                    &sequence
                )
            }
        }
        if status == statusQueueFull {
            return nil
        }
        try bindings.requireOK(status, operation: "logger_core_log")
        return sequence
    }

    @discardableResult
    public func trace(_ category: String, _ message: String) throws -> UInt64? {
        try log(.trace, category: category, message: message)
    }

    @discardableResult
    public func debug(_ category: String, _ message: String) throws -> UInt64? {
        try log(.debug, category: category, message: message)
    }

    @discardableResult
    public func info(_ category: String, _ message: String) throws -> UInt64? {
        try log(.info, category: category, message: message)
    }

    @discardableResult
    public func warn(_ category: String, _ message: String) throws -> UInt64? {
        try log(.warn, category: category, message: message)
    }

    @discardableResult
    public func error(_ category: String, _ message: String) throws -> UInt64? {
        try log(.error, category: category, message: message)
    }

    @discardableResult
    public func fatal(_ category: String, _ message: String) throws -> UInt64? {
        try log(.fatal, category: category, message: message)
    }

    public func poll() throws -> LoggerRecord? {
        try awaitNext(timeoutMs: 0)
    }

    public func awaitNext(timeoutMs: UInt32 = 1_000) throws -> LoggerRecord? {
        var category = [CChar](repeating: 0, count: Int(max(1, spec.categoryCapacity)))
        var message = [CChar](repeating: 0, count: Int(max(1, spec.messageCapacity)))
        var record = coakka_swift_logger_record_t()

        lock.lock()
        defer { lock.unlock() }
        guard !closed, let handle else {
            throw CoAkkaLoggerError.closed
        }

        let status = category.withUnsafeMutableBufferPointer { categoryBuffer in
            message.withUnsafeMutableBufferPointer { messageBuffer in
                coakka_swift_logger_read_next(
                    bindings.pointer,
                    handle,
                    timeoutMs,
                    categoryBuffer.baseAddress,
                    categoryBuffer.count,
                    messageBuffer.baseAddress,
                    messageBuffer.count,
                    &record
                )
            }
        }
        if status == statusTimedOut {
            return nil
        }
        try bindings.requireOK(status, operation: "logger_core_read_next")

        let categoryCount = min(Int(record.category_length), category.count)
        let messageCount = min(Int(record.message_length), message.count)
        let level = LoggerLevel(rawValue: record.level) ?? .info
        return LoggerRecord(
            sequence: record.sequence,
            wallTimeUnixMs: record.wall_time_unix_ms,
            monotonicTimeNs: record.monotonic_time_ns,
            level: level,
            levelName: bindings.levelName(record.level),
            category: decodeCStringBuffer(category, count: categoryCount),
            message: decodeCStringBuffer(message, count: messageCount)
        )
    }

    public func stats() throws -> LoggerStats {
        lock.lock()
        defer { lock.unlock() }
        guard !closed, let handle else {
            throw CoAkkaLoggerError.closed
        }

        var stats = coakka_swift_logger_stats_t()
        try bindings.requireOK(
            coakka_swift_logger_get_stats(bindings.pointer, handle, &stats),
            operation: "logger_core_get_stats"
        )
        return LoggerStats(
            state: stats.state,
            stateName: bindings.stateName(stats.state),
            queueCapacity: stats.queue_capacity,
            queueDepth: stats.queue_depth,
            queueHighWatermark: stats.queue_high_watermark,
            nextSequence: stats.next_sequence,
            emittedCount: stats.emitted_count,
            deliveredCount: stats.delivered_count,
            droppedCount: stats.dropped_count
        )
    }

    public func stop() throws {
        lock.lock()
        defer { lock.unlock() }
        if closed || stopped {
            return
        }
        guard let handle else {
            return
        }
        try bindings.requireOK(
            coakka_swift_logger_stop(bindings.pointer, handle),
            operation: "logger_core_stop"
        )
        stopped = true
    }

    public func close() throws {
        lock.lock()
        defer { lock.unlock() }
        if closed {
            return
        }
        if let handle {
            if !stopped {
                _ = coakka_swift_logger_stop(bindings.pointer, handle)
                stopped = true
            }
            coakka_swift_logger_destroy(bindings.pointer, handle)
            self.handle = nil
        }
        closed = true
    }
}

private final class NativeBindings {
    let pointer: OpaquePointer

    static func open(nativeLibraryPath: String?) throws -> NativeBindings {
        let path = try nativeLibraryPath ?? defaultNativeLibraryPath()
        guard FileManager.default.fileExists(atPath: path) else {
            throw CoAkkaLoggerError.nativeLibraryNotFound(path)
        }

        var pointer: OpaquePointer?
        let status = path.withCString { pathPtr in
            coakka_swift_logger_open(pathPtr, &pointer)
        }
        guard status == statusOK, let pointer else {
            throw CoAkkaLoggerError.nativeOpenFailed(path)
        }
        return NativeBindings(pointer: pointer)
    }

    private init(pointer: OpaquePointer) {
        self.pointer = pointer
    }

    deinit {
        coakka_swift_logger_close(pointer)
    }

    func readInfo() throws -> LoggerInfo {
        var info = coakka_swift_logger_info_t()
        try requireOK(
            coakka_swift_logger_get_info(pointer, &info),
            operation: "logger_core_get_info"
        )
        return LoggerInfo(
            abiVersion: info.abi_version,
            runtimeVersion: nativeString(info.runtime_version),
            gitCommit: nativeString(info.git_commit),
            docsHint: nativeString(info.docs_hint)
        )
    }

    func requireOK(_ status: Int32, operation: String) throws {
        guard status == statusOK else {
            throw CoAkkaLoggerError.nativeStatus(
                operation: operation,
                status: status,
                statusName: statusName(status)
            )
        }
    }

    func statusName(_ status: Int32) -> String {
        nativeString(coakka_swift_logger_status_name(pointer, status))
    }

    func levelName(_ level: Int32) -> String {
        nativeString(coakka_swift_logger_level_name(pointer, level))
    }

    func stateName(_ state: Int32) -> String {
        nativeString(coakka_swift_logger_state_name(pointer, state))
    }
}

private func defaultNativeLibraryPath() throws -> String {
    #if os(macOS) && arch(arm64)
    if let override = ProcessInfo.processInfo.environment["COAKKA_LOGGER_SWIFT_NATIVE_PATH"],
       !override.isEmpty {
        return override
    }
    guard let url = Bundle.module.url(
        forResource: "libcoakka_logger_core.10",
        withExtension: "dylib",
        subdirectory: "Resources/macos-aarch64"
    ) else {
        throw CoAkkaLoggerError.nativeLibraryNotFound("Resources/macos-aarch64/libcoakka_logger_core.10.dylib")
    }
    return url.path
    #else
    throw CoAkkaLoggerError.unsupportedPlatform(
        "coakka-logger-swift currently bundles macOS ARM64 native logger only"
    )
    #endif
}

private func nativeString(_ pointer: UnsafePointer<CChar>?) -> String {
    guard let pointer else {
        return ""
    }
    return String(cString: pointer)
}

private func decodeCStringBuffer(_ buffer: [CChar], count: Int) -> String {
    let bytes = buffer.prefix(count).map { UInt8(bitPattern: $0) }
    return String(decoding: bytes, as: UTF8.self)
}

private extension LoggerSpec {
    func normalized() -> LoggerSpec {
        LoggerSpec(
            systemName: systemName,
            queueCapacity: queueCapacity == 0 ? 256 : queueCapacity,
            categoryCapacity: categoryCapacity == 0 ? 64 : categoryCapacity,
            messageCapacity: messageCapacity == 0 ? 512 : messageCapacity,
            minLevel: minLevel
        )
    }
}
