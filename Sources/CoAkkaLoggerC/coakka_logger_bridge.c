#include "coakka_logger_bridge.h"

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

struct coakka_swift_logger_bindings {
  void* library;
  uint32_t (*get_abi_version)(void);
  coakka_logger_core_config_t (*default_config)(void);
  coakka_logger_status_t (*get_info)(coakka_logger_core_info_t*);
  coakka_logger_status_t (*create)(const coakka_logger_core_config_t*,
                                   coakka_logger_core_handle_t**);
  coakka_logger_status_t (*start)(coakka_logger_core_handle_t*);
  coakka_logger_status_t (*stop)(coakka_logger_core_handle_t*);
  void (*destroy)(coakka_logger_core_handle_t*);
  coakka_logger_status_t (*log)(coakka_logger_core_handle_t*,
                                coakka_logger_level_t,
                                const char*,
                                const char*,
                                uint64_t*);
  coakka_logger_status_t (*read_next)(coakka_logger_core_handle_t*,
                                      uint32_t,
                                      coakka_logger_core_record_buffer_t*);
  coakka_logger_status_t (*get_stats)(coakka_logger_core_handle_t*,
                                      coakka_logger_core_stats_t*);
  int (*is_enabled_for_category)(coakka_logger_core_handle_t*,
                                 const char*,
                                 coakka_logger_level_t);
  const char* (*status_name)(coakka_logger_status_t);
  const char* (*level_name)(coakka_logger_level_t);
  const char* (*state_name)(coakka_logger_core_state_t);
};

static void* coakka_swift_symbol(void* library, const char* name, int* ok) {
  void* symbol = dlsym(library, name);
  if (symbol == NULL) {
    *ok = 0;
  }
  return symbol;
}

int32_t coakka_swift_logger_open(const char* native_path,
                                 coakka_swift_logger_bindings_t** out_bindings) {
  if (native_path == NULL || out_bindings == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }

  *out_bindings = NULL;
  void* library = dlopen(native_path, RTLD_NOW | RTLD_LOCAL);
  if (library == NULL) {
    return COAKKA_LOGGER_STATUS_INTERNAL_ERROR;
  }

  coakka_swift_logger_bindings_t* bindings =
      (coakka_swift_logger_bindings_t*)calloc(1, sizeof(coakka_swift_logger_bindings_t));
  if (bindings == NULL) {
    dlclose(library);
    return COAKKA_LOGGER_STATUS_INTERNAL_ERROR;
  }
  bindings->library = library;

  int ok = 1;
  bindings->get_abi_version = coakka_swift_symbol(library, "coakka_logger_core_get_abi_version", &ok);
  bindings->default_config = coakka_swift_symbol(library, "coakka_logger_core_default_config", &ok);
  bindings->get_info = coakka_swift_symbol(library, "coakka_logger_core_get_info", &ok);
  bindings->create = coakka_swift_symbol(library, "coakka_logger_core_create", &ok);
  bindings->start = coakka_swift_symbol(library, "coakka_logger_core_start", &ok);
  bindings->stop = coakka_swift_symbol(library, "coakka_logger_core_stop", &ok);
  bindings->destroy = coakka_swift_symbol(library, "coakka_logger_core_destroy", &ok);
  bindings->log = coakka_swift_symbol(library, "coakka_logger_core_log", &ok);
  bindings->read_next = coakka_swift_symbol(library, "coakka_logger_core_read_next", &ok);
  bindings->get_stats = coakka_swift_symbol(library, "coakka_logger_core_get_stats", &ok);
  bindings->is_enabled_for_category =
      coakka_swift_symbol(library, "coakka_logger_core_is_enabled_for_category", &ok);
  bindings->status_name = coakka_swift_symbol(library, "coakka_logger_status_name", &ok);
  bindings->level_name = coakka_swift_symbol(library, "coakka_logger_level_name", &ok);
  bindings->state_name = coakka_swift_symbol(library, "coakka_logger_state_name", &ok);

  if (!ok) {
    dlclose(library);
    free(bindings);
    return COAKKA_LOGGER_STATUS_INTERNAL_ERROR;
  }

  *out_bindings = bindings;
  return COAKKA_LOGGER_STATUS_OK;
}

void coakka_swift_logger_close(coakka_swift_logger_bindings_t* bindings) {
  if (bindings == NULL) {
    return;
  }
  if (bindings->library != NULL) {
    dlclose(bindings->library);
  }
  free(bindings);
}

uint32_t coakka_swift_logger_get_abi_version(coakka_swift_logger_bindings_t* bindings) {
  if (bindings == NULL || bindings->get_abi_version == NULL) {
    return 0;
  }
  return bindings->get_abi_version();
}

int32_t coakka_swift_logger_get_info(coakka_swift_logger_bindings_t* bindings,
                                     coakka_swift_logger_info_t* out_info) {
  if (bindings == NULL || out_info == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  coakka_logger_core_info_t info;
  memset(&info, 0, sizeof(info));
  info.struct_size = sizeof(info);
  coakka_logger_status_t status = bindings->get_info(&info);
  if (status != COAKKA_LOGGER_STATUS_OK) {
    return status;
  }
  out_info->abi_version = info.abi_version;
  out_info->runtime_version = info.runtime_version;
  out_info->git_commit = info.git_commit;
  out_info->docs_hint = info.docs_hint;
  return COAKKA_LOGGER_STATUS_OK;
}

int32_t coakka_swift_logger_create(coakka_swift_logger_bindings_t* bindings,
                                   const char* system_name,
                                   uint32_t queue_capacity,
                                   uint32_t category_capacity,
                                   uint32_t message_capacity,
                                   int32_t min_level,
                                   coakka_logger_core_handle_t** out_handle) {
  if (bindings == NULL || out_handle == NULL || system_name == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  coakka_logger_core_config_t config = bindings->default_config();
  config.system_name = system_name;
  config.queue_capacity = queue_capacity;
  config.category_capacity = category_capacity;
  config.message_capacity = message_capacity;
  config.min_level = (coakka_logger_level_t)min_level;
  return bindings->create(&config, out_handle);
}

int32_t coakka_swift_logger_start(coakka_swift_logger_bindings_t* bindings,
                                  coakka_logger_core_handle_t* handle) {
  if (bindings == NULL || handle == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  return bindings->start(handle);
}

int32_t coakka_swift_logger_stop(coakka_swift_logger_bindings_t* bindings,
                                 coakka_logger_core_handle_t* handle) {
  if (bindings == NULL || handle == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  return bindings->stop(handle);
}

void coakka_swift_logger_destroy(coakka_swift_logger_bindings_t* bindings,
                                 coakka_logger_core_handle_t* handle) {
  if (bindings == NULL || handle == NULL) {
    return;
  }
  bindings->destroy(handle);
}

int32_t coakka_swift_logger_log(coakka_swift_logger_bindings_t* bindings,
                                coakka_logger_core_handle_t* handle,
                                int32_t level,
                                const char* category,
                                const char* message,
                                uint64_t* out_sequence) {
  if (bindings == NULL || handle == NULL || category == NULL || message == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  return bindings->log(handle, (coakka_logger_level_t)level, category, message, out_sequence);
}

int32_t coakka_swift_logger_read_next(coakka_swift_logger_bindings_t* bindings,
                                      coakka_logger_core_handle_t* handle,
                                      uint32_t timeout_ms,
                                      char* category,
                                      size_t category_capacity,
                                      char* message,
                                      size_t message_capacity,
                                      coakka_swift_logger_record_t* out_record) {
  if (bindings == NULL || handle == NULL || category == NULL || message == NULL ||
      out_record == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  coakka_logger_core_record_buffer_t record;
  memset(&record, 0, sizeof(record));
  record.struct_size = sizeof(record);
  record.category = category;
  record.category_capacity = category_capacity;
  record.message = message;
  record.message_capacity = message_capacity;

  coakka_logger_status_t status = bindings->read_next(handle, timeout_ms, &record);
  if (status != COAKKA_LOGGER_STATUS_OK) {
    return status;
  }

  out_record->sequence = record.sequence;
  out_record->wall_time_unix_ms = record.wall_time_unix_ms;
  out_record->monotonic_time_ns = record.monotonic_time_ns;
  out_record->level = (int32_t)record.level;
  out_record->category_length = record.category_length;
  out_record->message_length = record.message_length;
  return COAKKA_LOGGER_STATUS_OK;
}

int32_t coakka_swift_logger_get_stats(coakka_swift_logger_bindings_t* bindings,
                                      coakka_logger_core_handle_t* handle,
                                      coakka_swift_logger_stats_t* out_stats) {
  if (bindings == NULL || handle == NULL || out_stats == NULL) {
    return COAKKA_LOGGER_STATUS_INVALID_ARGUMENT;
  }
  coakka_logger_core_stats_t stats;
  memset(&stats, 0, sizeof(stats));
  stats.struct_size = sizeof(stats);
  coakka_logger_status_t status = bindings->get_stats(handle, &stats);
  if (status != COAKKA_LOGGER_STATUS_OK) {
    return status;
  }
  out_stats->state = (int32_t)stats.state;
  out_stats->queue_capacity = stats.queue_capacity;
  out_stats->queue_depth = stats.queue_depth;
  out_stats->queue_high_watermark = stats.queue_high_watermark;
  out_stats->next_sequence = stats.next_sequence;
  out_stats->emitted_count = stats.emitted_count;
  out_stats->delivered_count = stats.delivered_count;
  out_stats->dropped_count = stats.dropped_count;
  return COAKKA_LOGGER_STATUS_OK;
}

int32_t coakka_swift_logger_is_enabled_for_category(coakka_swift_logger_bindings_t* bindings,
                                                    coakka_logger_core_handle_t* handle,
                                                    const char* category,
                                                    int32_t level) {
  if (bindings == NULL || handle == NULL || category == NULL) {
    return 0;
  }
  return bindings->is_enabled_for_category(handle, category, (coakka_logger_level_t)level);
}

const char* coakka_swift_logger_status_name(coakka_swift_logger_bindings_t* bindings,
                                            int32_t status) {
  if (bindings == NULL || bindings->status_name == NULL) {
    return "UNKNOWN";
  }
  return bindings->status_name((coakka_logger_status_t)status);
}

const char* coakka_swift_logger_level_name(coakka_swift_logger_bindings_t* bindings,
                                           int32_t level) {
  if (bindings == NULL || bindings->level_name == NULL) {
    return "UNKNOWN";
  }
  return bindings->level_name((coakka_logger_level_t)level);
}

const char* coakka_swift_logger_state_name(coakka_swift_logger_bindings_t* bindings,
                                           int32_t state) {
  if (bindings == NULL || bindings->state_name == NULL) {
    return "UNKNOWN";
  }
  return bindings->state_name((coakka_logger_core_state_t)state);
}
