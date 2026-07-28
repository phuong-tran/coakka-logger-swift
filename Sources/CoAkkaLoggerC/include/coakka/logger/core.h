#ifndef COAKKA_LOGGER_CORE_H
#define COAKKA_LOGGER_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COAKKA_LOGGER_CORE_ABI_VERSION 10u

typedef enum coakka_logger_status_t {
  COAKKA_LOGGER_STATUS_OK = 0,
  COAKKA_LOGGER_STATUS_INVALID_ARGUMENT = 1,
  COAKKA_LOGGER_STATUS_BAD_STATE = 2,
  COAKKA_LOGGER_STATUS_QUEUE_FULL = 3,
  COAKKA_LOGGER_STATUS_RECORD_TOO_LARGE = 4,
  COAKKA_LOGGER_STATUS_TIMED_OUT = 5,
  COAKKA_LOGGER_STATUS_BUFFER_TOO_SMALL = 6,
  COAKKA_LOGGER_STATUS_INTERNAL_ERROR = 7
} coakka_logger_status_t;

typedef enum coakka_logger_level_t {
  COAKKA_LOGGER_LEVEL_TRACE = 0,
  COAKKA_LOGGER_LEVEL_DEBUG = 1,
  COAKKA_LOGGER_LEVEL_INFO = 2,
  COAKKA_LOGGER_LEVEL_WARN = 3,
  COAKKA_LOGGER_LEVEL_ERROR = 4,
  COAKKA_LOGGER_LEVEL_FATAL = 5
} coakka_logger_level_t;

typedef enum coakka_logger_level_mask_flags_t {
  COAKKA_LOGGER_LEVEL_MASK_TRACE = 1u << COAKKA_LOGGER_LEVEL_TRACE,
  COAKKA_LOGGER_LEVEL_MASK_DEBUG = 1u << COAKKA_LOGGER_LEVEL_DEBUG,
  COAKKA_LOGGER_LEVEL_MASK_INFO = 1u << COAKKA_LOGGER_LEVEL_INFO,
  COAKKA_LOGGER_LEVEL_MASK_WARN = 1u << COAKKA_LOGGER_LEVEL_WARN,
  COAKKA_LOGGER_LEVEL_MASK_ERROR = 1u << COAKKA_LOGGER_LEVEL_ERROR,
  COAKKA_LOGGER_LEVEL_MASK_FATAL = 1u << COAKKA_LOGGER_LEVEL_FATAL,
  COAKKA_LOGGER_LEVEL_MASK_ALL = (1u << 6) - 1u
} coakka_logger_level_mask_flags_t;

typedef enum coakka_logger_core_state_t {
  COAKKA_LOGGER_CORE_STATE_CREATED = 0,
  COAKKA_LOGGER_CORE_STATE_STARTED = 1,
  COAKKA_LOGGER_CORE_STATE_STOPPED = 2
} coakka_logger_core_state_t;

typedef enum coakka_logger_sink_mode_t {
  COAKKA_LOGGER_SINK_MODE_MANUAL_DRAIN = 0,
  COAKKA_LOGGER_SINK_MODE_FILE = 1,
  COAKKA_LOGGER_SINK_MODE_CONSOLE = 2,
  COAKKA_LOGGER_SINK_MODE_MULTI = 3
} coakka_logger_sink_mode_t;

typedef enum coakka_logger_sink_target_flags_t {
  COAKKA_LOGGER_SINK_TARGET_NONE = 0u,
  COAKKA_LOGGER_SINK_TARGET_FILE = 1u << 0,
  COAKKA_LOGGER_SINK_TARGET_CONSOLE = 1u << 1
} coakka_logger_sink_target_flags_t;

typedef enum coakka_logger_pressure_state_t {
  COAKKA_LOGGER_PRESSURE_STATE_NORMAL = 0,
  COAKKA_LOGGER_PRESSURE_STATE_QUOTA_PRESSURE = 1,
  COAKKA_LOGGER_PRESSURE_STATE_DROP_LOW_PRIORITY = 2,
  COAKKA_LOGGER_PRESSURE_STATE_EMERGENCY_ONLY = 3
} coakka_logger_pressure_state_t;

typedef enum coakka_logger_emergency_reason_t {
  COAKKA_LOGGER_EMERGENCY_REASON_NONE = 0,
  COAKKA_LOGGER_EMERGENCY_REASON_QUOTA = 1,
  COAKKA_LOGGER_EMERGENCY_REASON_SINK_WRITE_FAILURE = 2,
  COAKKA_LOGGER_EMERGENCY_REASON_SINK_REOPEN_FAILURE = 3,
  COAKKA_LOGGER_EMERGENCY_REASON_SINK_ROLL_FAILURE = 4,
  COAKKA_LOGGER_EMERGENCY_REASON_SINK_APPEND_FAILURE = 5
} coakka_logger_emergency_reason_t;

typedef struct coakka_logger_core_handle coakka_logger_core_handle_t;

typedef struct coakka_logger_core_info_t {
  size_t struct_size;
  uint32_t abi_version;
  const char* runtime_version;
  const char* git_commit;
  const char* docs_hint;
} coakka_logger_core_info_t;

typedef struct coakka_logger_core_config_t {
  size_t struct_size;
  const char* system_name;
  uint32_t queue_capacity;
  coakka_logger_level_t min_level;
  coakka_logger_sink_mode_t sink_mode;
  const char* output_path;
  uint64_t max_file_size_bytes;
  uint64_t max_total_size_bytes;
  coakka_logger_pressure_state_t pressure_state;
  uint32_t max_archived_files;
  uint32_t category_capacity;
  uint32_t message_capacity;
  const char* emergency_output_path;
  uint64_t emergency_file_size_bytes;
  uint32_t category_override_capacity;
  uint32_t sink_targets;
  int32_t console_fd;
  int32_t console_stdout_fd;
  int32_t console_stderr_fd;
  uint32_t console_stdout_level_mask;
  uint32_t console_stderr_level_mask;
  const char* file_output_path;
  uint64_t file_max_size_bytes;
  uint64_t file_total_size_limit_bytes;
  uint32_t file_max_archived_files;
  const char* file_emergency_output_path;
  uint64_t file_emergency_size_bytes;
} coakka_logger_core_config_t;

typedef struct coakka_logger_core_config_view_t {
  size_t struct_size;
  const char* system_name;
  coakka_logger_core_state_t state;
  uint32_t queue_capacity;
  coakka_logger_level_t min_level;
  coakka_logger_sink_mode_t sink_mode;
  const char* output_path;
  uint64_t max_file_size_bytes;
  uint64_t max_total_size_bytes;
  coakka_logger_pressure_state_t pressure_state;
  uint32_t max_archived_files;
  uint32_t category_capacity;
  uint32_t message_capacity;
  const char* emergency_output_path;
  uint64_t emergency_file_size_bytes;
  uint32_t category_override_capacity;
  uint32_t sink_targets;
  int32_t console_fd;
  int32_t console_stdout_fd;
  int32_t console_stderr_fd;
  uint32_t console_stdout_level_mask;
  uint32_t console_stderr_level_mask;
  const char* file_output_path;
  uint64_t file_max_size_bytes;
  uint64_t file_total_size_limit_bytes;
  uint32_t file_max_archived_files;
  const char* file_emergency_output_path;
  uint64_t file_emergency_size_bytes;
} coakka_logger_core_config_view_t;

typedef struct coakka_logger_core_stats_t {
  size_t struct_size;
  coakka_logger_core_state_t state;
  uint32_t queue_capacity;
  uint32_t queue_depth;
  uint32_t queue_high_watermark;
  uint64_t next_sequence;
  uint64_t emitted_count;
  uint64_t delivered_count;
  uint64_t dropped_count;
  uint64_t dropped_below_level_count;
  uint64_t sink_write_count;
  uint64_t sink_bytes_written;
  uint64_t sink_write_failure_count;
  uint64_t sink_current_file_size_bytes;
  uint64_t sink_total_size_bytes;
  coakka_logger_pressure_state_t pressure_state;
  uint64_t sink_roll_count;
  uint64_t sink_deleted_archive_count;
  uint64_t dropped_pressure_count;
  uint32_t emergency_active;
  coakka_logger_emergency_reason_t emergency_last_reason;
  uint64_t emergency_enter_count;
  uint64_t emergency_recovered_count;
  uint64_t emergency_bytes_written;
  uint64_t emergency_write_failure_count;
  uint64_t emergency_overwrite_count;
  coakka_logger_emergency_reason_t emergency_last_recovered_reason;
  uint64_t emergency_last_recovered_duration_ms;
  uint64_t emergency_last_recovered_dropped_total;
  uint64_t emergency_last_recovered_dropped_pressure;
  uint64_t emergency_last_recovered_deleted_archives;
  uint64_t reload_count;
  uint32_t category_override_count;
  uint64_t sink_reopen_failure_count;
  uint64_t sink_roll_failure_count;
  uint64_t sink_append_failure_count;
  uint64_t sink_cleanup_failure_count;
  uint64_t file_write_count;
  uint64_t file_bytes_written;
  uint64_t file_write_failure_count;
  uint64_t file_current_size_bytes;
  uint64_t file_total_size_bytes;
  uint64_t file_roll_count;
  uint64_t file_deleted_archive_count;
  uint64_t file_reopen_failure_count;
  uint64_t file_roll_failure_count;
  uint64_t file_append_failure_count;
  uint64_t file_cleanup_failure_count;
  uint64_t console_write_count;
  uint64_t console_bytes_written;
  uint64_t console_write_failure_count;
} coakka_logger_core_stats_t;

/* Raw record path for callers that need explicit timestamp ownership.
 * `category` and `message` may be `NULL` and are treated as empty strings.
 * When `wall_time_unix_ms` or `monotonic_time_ns` is `0`, `emit(...)` fills
 * that timestamp from the runtime clock before enqueue. */
typedef struct coakka_logger_core_record_t {
  size_t struct_size;
  coakka_logger_level_t level;
  const char* category;
  const char* message;
  uint64_t wall_time_unix_ms;
  uint64_t monotonic_time_ns;
} coakka_logger_core_record_t;

typedef struct coakka_logger_core_record_buffer_t {
  size_t struct_size;
  uint64_t sequence;
  uint64_t wall_time_unix_ms;
  uint64_t monotonic_time_ns;
  coakka_logger_level_t level;
  char* category;
  size_t category_capacity;
  size_t category_length;
  char* message;
  size_t message_capacity;
  size_t message_length;
} coakka_logger_core_record_buffer_t;

uint32_t coakka_logger_core_get_abi_version(void);
coakka_logger_core_config_t coakka_logger_core_default_config(void);
coakka_logger_status_t coakka_logger_core_get_info(coakka_logger_core_info_t* out_info);
coakka_logger_status_t coakka_logger_core_create(const coakka_logger_core_config_t* config,
                                                 coakka_logger_core_handle_t** out_handle);
coakka_logger_status_t coakka_logger_core_start(coakka_logger_core_handle_t* handle);
coakka_logger_status_t coakka_logger_core_stop(coakka_logger_core_handle_t* handle);
void coakka_logger_core_destroy(coakka_logger_core_handle_t* handle);
coakka_logger_status_t coakka_logger_core_get_config(coakka_logger_core_handle_t* handle,
                                                     coakka_logger_core_config_view_t* out_view);
coakka_logger_status_t coakka_logger_core_get_stats(coakka_logger_core_handle_t* handle,
                                                    coakka_logger_core_stats_t* out_stats);
coakka_logger_status_t coakka_logger_core_reload_config(
    coakka_logger_core_handle_t* handle,
    const coakka_logger_core_config_t* config);
/* Fast enable probes for already-known enum levels. Invalid levels fail closed
 * to `0` instead of being interpreted. */
int coakka_logger_core_is_enabled(coakka_logger_core_handle_t* handle,
                                  coakka_logger_level_t level);
int coakka_logger_core_is_enabled_for_category(coakka_logger_core_handle_t* handle,
                                               const char* category,
                                               coakka_logger_level_t level);
coakka_logger_status_t coakka_logger_core_set_category_level(coakka_logger_core_handle_t* handle,
                                                             const char* category,
                                                             coakka_logger_level_t min_level);
coakka_logger_status_t coakka_logger_core_clear_category_level(coakka_logger_core_handle_t* handle,
                                                               const char* category);
/* Normal caller-facing text logging path. `category` and `message` may be
 * `NULL` and are treated as empty. `level` must stay inside the public
 * TRACE..FATAL range. */
coakka_logger_status_t coakka_logger_core_log(coakka_logger_core_handle_t* handle,
                                              coakka_logger_level_t level,
                                              const char* category,
                                              const char* message,
                                              uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_trace(coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    const char* message,
                                                    uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_debug(coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    const char* message,
                                                    uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_info(coakka_logger_core_handle_t* handle,
                                                   const char* category,
                                                   const char* message,
                                                   uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_warn(coakka_logger_core_handle_t* handle,
                                                   const char* category,
                                                   const char* message,
                                                   uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_error(coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    const char* message,
                                                    uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_log_fatal(coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    const char* message,
                                                    uint64_t* out_sequence);
/* Advanced raw path when the caller wants to provide a full record directly.
 * `record->level` must stay inside the public TRACE..FATAL range. */
coakka_logger_status_t coakka_logger_core_emit(coakka_logger_core_handle_t* handle,
                                               const coakka_logger_core_record_t* record,
                                               uint64_t* out_sequence);
coakka_logger_status_t coakka_logger_core_read_next(
    coakka_logger_core_handle_t* handle,
    uint32_t timeout_ms,
    coakka_logger_core_record_buffer_t* out_record);

#ifdef __cplusplus
}
#endif

#endif
