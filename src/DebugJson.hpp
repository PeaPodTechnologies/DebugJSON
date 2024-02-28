#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef DEBUG_SERIAL
#ifdef HAVE_HWSERIAL0
#define DEBUG_SERIAL Serial
#elif
#error DEBUG_SERIAL is not defined
#endif
#endif

#define DEBUG_JSON_SIZE 100

#define DEBUG_BREAKPOINT() DebugJson::debug(DEBUG_INFO, __FILE__ ":" __LINE__ ":" __FUNCTION__)

namespace DebugJson {
  typedef enum {
    DEBUG_NONE = 0, // Revision and heartbeat
    DEBUG_INFO = 1,
    DEBUG_WARN = 2,
    DEBUG_ERROR = 3,
    TELEMETRY = 4,
    COMMAND = 5,
    CONFIGURATION = 6,
  } msgtype_t;

  void debug(const msgtype_t& type, const char *msg = nullptr);

  void revision(const uint8_t& version);

  // Looks for [key: string]: number
  void telemetry(JsonObject& data);

  void heartbeat(void);
};