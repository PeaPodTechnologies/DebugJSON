#ifndef DEBUG_JSON_H_
#define DEBUG_JSON_H_

#include <Arduino.h>
#include <ArduinoJson.h>

#ifndef DEBUG_SERIAL
// #ifdef HAVE_HWSERIAL0
#define DEBUG_SERIAL Serial
// #else
// #error DEBUG_SERIAL is not defined
// #endif
#endif

#define DEBUG_JSON_SIZE_DOC 128
#define DEBUG_JSON_SIZE_BUFFER SERIAL_TX_BUFFER_SIZE
#define DEBUG_JSON_HEARTBEAT 5000
#define DEBUG_JSON_REFRESH_MICROS 1000 // 1ms

#define DEBUG_JSON_POSTPRINT_MICROS 100 // 1ms - Uncomment to add a delay after printing
// #define DEBUG_JSON_SRAM 1

// #define DEBUG_JSON_LEVEL_MIN DebugJson::DEBUG_INFO
#define DEBUG_JSON_LEVEL_MIN DebugJson::DEBUG_NONE

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

  // Used during operations between all functions
  extern char bufferTx[];
  extern char bufferRx[];
  extern JsonDocument docTx, docRx;

  const __FlashStringHelper* parseType(const msgtype_t& type);

  #ifdef DEBUG_SERIAL

    size_t jsonPrint(Stream& out = DEBUG_SERIAL);
    size_t jsonPrintln(Stream& out = DEBUG_SERIAL);

    void revision(const uint8_t& version, Stream& out = DEBUG_SERIAL);

    // Looks for [key: string]: number
    void telemetry(unsigned long timestamp, JsonObject data, Stream& out = DEBUG_SERIAL);
    template <typename T> void batchTelemetry(unsigned long timestamp, String key, T value, bool send, Stream& out = DEBUG_SERIAL);
    template <typename T> void telemetry(unsigned long timestamp, T value, Stream& out = DEBUG_SERIAL);

    void heartbeat(unsigned long timestamp, Stream& out = DEBUG_SERIAL);
    void heartbeat(Stream& out = DEBUG_SERIAL);
  
  #else

    size_t jsonPrint(Stream& out);
    size_t jsonPrintln(Stream& out);

    void revision(const uint8_t& version, Stream& out);

    // Looks for [key: string]: number
    void telemetry(unsigned long timestamp, JsonObject data, Stream& out);
    template <typename T> void batchTelemetry(unsigned long timestamp, String key, T value, bool send, Stream& out);
    template <typename T> void telemetry(unsigned long timestamp, T value, Stream& out);

    void heartbeat(unsigned long timestamp, Stream& out);
    void heartbeat(Stream& out);
  
  #endif

  typedef void (*debugjson_cb_config_t)(JsonObject config);
  typedef void (*debugjson_cb_commands_t)(JsonObject commands);

  #ifdef DEBUG_SERIAL

  void update(Stream& serial = DEBUG_SERIAL, debugjson_cb_commands_t cb_commands = nullptr, debugjson_cb_config_t cb_config = nullptr);
  void fixedUpdate(unsigned long timestamp, Stream& serial = DEBUG_SERIAL);

  #else

  void update(Stream& Serial, debugjson_cb_commands_t cb_commands = nullptr, debugjson_cb_config_t cb_config = nullptr);
  void fixedUpdate(unsigned long timestamp, Stream& Serial);

  #endif

  template <msgtype_t T> class DebugStream : public Stream {
    private:
      const msgtype_t type = T;
      Stream& out; // Wrapped
    public:
      DebugStream(Stream& s) : out(s) {}
      size_t write(const uint8_t *buffer, size_t size) override;
      size_t write(uint8_t b) override { return out.write(b); } // Leave as-is (i.e. delineators)
      int availableForWrite() override { return out.availableForWrite(); } // Leave as-is
      void flush() override { out.flush(); } // Leave as-is
      int available() { return out.available(); } // Leave as-is
      int read() { return out.read(); } // Leave as-is
      int peek() { return out.peek(); } // Leave as-is
  };

};


#ifdef DEBUG_SERIAL
  extern DebugJson::DebugStream<DebugJson::DEBUG_NONE> DebugJsonBreakpoints; // I.e. no error, breakpoints
  extern DebugJson::DebugStream<DebugJson::DEBUG_INFO> DebugJsonOut; // I.e. no error, breakpoints
  extern DebugJson::DebugStream<DebugJson::DEBUG_WARN> DebugJsonWarning; // I.e. software glitch
  extern DebugJson::DebugStream<DebugJson::DEBUG_ERROR> DebugJsonError; // I.e. hardware failure
  
  #define DEBUG_JSON(...)   DebugJsonOut.println(__VA_ARGS__)
  #define WARNING_JSON(...) DebugJsonWarning.println(__VA_ARGS__)
  #define ERROR_JSON(...)   DebugJsonError.println(__VA_ARGS__)
  #define BP_JSON()         DebugJsonBreakpoints.println((String(__FILE__) + ':' + String(__LINE__)))
#endif

#include "DebugJson.tpp"

#endif