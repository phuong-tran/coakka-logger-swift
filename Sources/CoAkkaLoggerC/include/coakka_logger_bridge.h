#ifndef COAKKA_LOGGER_SWIFT_BRIDGE_H
#define COAKKA_LOGGER_SWIFT_BRIDGE_H

#include <stddef.h>
#include <stdint.h>

#include "coakka/logger/core.h"
#include "coakka/logger/utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct coakka_swift_logger_bindings coakka_swift_logger_bindings_t;

typedef struct coakka_swift_logger_info_t {
  uint32_t abi_version;
  const char* runtime_version;
  const char* git_commit;
  const char* docs_hint;
} coakka_swift_logger_info_t;

typedef struct coakka_swift_logger_stats_t {
  int32_t state;
  uint32_t queue_capacity;
  uint32_t queue_depth;
  uint32_t queue_high_watermark;
  uint64_t next_sequence;
  uint64_t emitted_count;
  uint64_t delivered_count;
  uint64_t dropped_count;
} coakka_swift_logger_stats_t;

typedef struct coakka_swift_logger_record_t {
  uint64_t sequence;
  uint64_t wall_time_unix_ms;
  uint64_t monotonic_time_ns;
  int32_t level;
  size_t category_length;
  size_t message_length;
} coakka_swift_logger_record_t;

int32_t coakka_swift_logger_open(const char* native_path,
                                 coakka_swift_logger_bindings_t** out_bindings);
void coakka_swift_logger_close(coakka_swift_logger_bindings_t* bindings);

uint32_t coakka_swift_logger_get_abi_version(coakka_swift_logger_bindings_t* bindings);
int32_t coakka_swift_logger_get_info(coakka_swift_logger_bindings_t* bindings,
                                     coakka_swift_logger_info_t* out_info);
int32_t coakka_swift_logger_create(coakka_swift_logger_bindings_t* bindings,
                                   const char* system_name,
                                   uint32_t queue_capacity,
                                   uint32_t category_capacity,
                                   uint32_t message_capacity,
                                   int32_t min_level,
                                   coakka_logger_core_handle_t** out_handle);
int32_t coakka_swift_logger_start(coakka_swift_logger_bindings_t* bindings,
                                  coakka_logger_core_handle_t* handle);
int32_t coakka_swift_logger_stop(coakka_swift_logger_bindings_t* bindings,
                                 coakka_logger_core_handle_t* handle);
void coakka_swift_logger_destroy(coakka_swift_logger_bindings_t* bindings,
                                 coakka_logger_core_handle_t* handle);
int32_t coakka_swift_logger_log(coakka_swift_logger_bindings_t* bindings,
                                coakka_logger_core_handle_t* handle,
                                int32_t level,
                                const char* category,
                                const char* message,
                                uint64_t* out_sequence);
int32_t coakka_swift_logger_read_next(coakka_swift_logger_bindings_t* bindings,
                                      coakka_logger_core_handle_t* handle,
                                      uint32_t timeout_ms,
                                      char* category,
                                      size_t category_capacity,
                                      char* message,
                                      size_t message_capacity,
                                      coakka_swift_logger_record_t* out_record);
int32_t coakka_swift_logger_get_stats(coakka_swift_logger_bindings_t* bindings,
                                      coakka_logger_core_handle_t* handle,
                                      coakka_swift_logger_stats_t* out_stats);
int32_t coakka_swift_logger_is_enabled_for_category(coakka_swift_logger_bindings_t* bindings,
                                                    coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    int32_t level);
const char* coakka_swift_logger_status_name(coakka_swift_logger_bindings_t* bindings,
                                            int32_t status);
const char* coakka_swift_logger_level_name(coakka_swift_logger_bindings_t* bindings,
                                           int32_t level);
const char* coakka_swift_logger_state_name(coakka_swift_logger_bindings_t* bindings,
                                           int32_t state);

#ifdef __cplusplus
}
#endif

#endif
