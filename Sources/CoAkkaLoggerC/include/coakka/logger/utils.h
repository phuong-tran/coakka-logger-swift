#ifndef COAKKA_LOGGER_UTILS_H
#define COAKKA_LOGGER_UTILS_H

#include "coakka/logger/core.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* coakka_logger_status_name(coakka_logger_status_t status);
const char* coakka_logger_level_name(coakka_logger_level_t level);
const char* coakka_logger_state_name(coakka_logger_core_state_t state);
const char* coakka_logger_sink_mode_name(coakka_logger_sink_mode_t mode);
const char* coakka_logger_pressure_state_name(coakka_logger_pressure_state_t state);
const char* coakka_logger_emergency_reason_name(coakka_logger_emergency_reason_t reason);

#ifdef __cplusplus
}
#endif

#endif
