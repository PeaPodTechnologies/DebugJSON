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

#define DEBUG_JSON_SIZE_DOC 256
#define DEBUG_JSON_SIZE_BUFFER SERIAL_TX_BUFFER_SIZE
#define DEBUG_JSON_HEARTBEAT 1000
#define DEBUG_JSON_REFRESH_MICROS 1000 // 1ms

#define DEBUG_JSON_POSTPRINT_MICROS 100 // 1ms - Uncomment to add a delay after printing
#define DEBUG_JSON_SRAM 1

#define DEBUG_JSON_LEVEL_MIN DebugJson::DEBUG_INFO

namespace DebugJson {
  typedef enum {
    DEBUG_NONE = 0, // Heartbeat and verbose debugging
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_ERROR,
    EVENT_TELEMETRY,
    EVENT_COMMAND,
    EVENT_CONFIGURATION,
    EVENT_REVISION
  } msgtype_t;

  void jsonPrint(Stream& out = DEBUG_SERIAL);
  void jsonPrint(const String& msg, Stream& out = DEBUG_SERIAL);
  void jsonPrint(const char* msg, size_t len, Stream& out = DEBUG_SERIAL);
  void jsonPrint(const __FlashStringHelper* msg, size_t len, Stream& out = DEBUG_SERIAL);
  void jsonPrint(unsigned long timestamp, Stream& out = DEBUG_SERIAL);
  void jsonPrint(unsigned long timestamp, const String& msg, Stream& out = DEBUG_SERIAL);
  void jsonPrint(unsigned long timestamp, const char* msg, size_t len, Stream& out = DEBUG_SERIAL);
  void jsonPrint(unsigned long timestamp, const __FlashStringHelper* msg, size_t len, Stream& out = DEBUG_SERIAL);

  void debug(const msgtype_t& type, const String& msg);
  void debug(const String& msg);
  void debug(const msgtype_t& type, const char* msg, size_t len);
  void debug(const msgtype_t& type, const __FlashStringHelper* msg, size_t len);
  void debug(const char* msg, size_t len);
  void debug(const __FlashStringHelper* msg, size_t len);

  void revision(const uint8_t& version);

  // Looks for [key: string]: number
  void telemetry(unsigned long timestamp, JsonObject& data);
  template <typename T> void batchTelemetry(unsigned long timestamp, String key, T value, bool send = false);
  template <typename T> void telemetry(unsigned long timestamp, T value);

  void heartbeat(unsigned long timestamp);

  // typedef void (*debugjson_cb_config_t)(JsonDocument config);
  // typedef void (*debugjson_cb_commands_t)(JsonDocument commands);

  // void update(debugjson_cb_commands_t cb_commands = nullptr, debugjson_cb_config_t cb_config = nullptr);
  // void fixedUpdate(unsigned long timestamp);
};

#define DEBUG_JSON(...) DebugJson::debug(DebugJson::DEBUG_NONE, __VA_ARGS__)
// #define DEBUG_JSON(m, len)   DebugJson::debug(DebugJson::DEBUG_INFO, m, len)
#define WARNING_JSON(...) DebugJson::debug(DebugJson::DEBUG_WARN, __VA_ARGS__)
#define ERROR_JSON(...)   DebugJson::debug(DebugJson::DEBUG_ERROR, __VA_ARGS__)
#define BP_JSON()       DEBUG_JSON(String(__PRETTY_FUNCTION__) + ':' + String(__LINE__))