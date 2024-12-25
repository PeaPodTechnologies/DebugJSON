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
#define DEBUG_JSON_SIZE_STRINGWRITER 256

#define DEBUG_JSON_HEARTBEAT 5000
#define DEBUG_JSON_REFRESH_MICROS 1000 // 1ms

#define DEBUG_JSON_POSTPRINT_MICROS 100 // 1ms - Uncomment to add a delay after printing
// #define DEBUG_JSON_SRAM 1

// #define DEBUG_JSON_MSG_SEP "; "
#define DEBUG_JSON_MSG_SEP ""
#define DEBUG_JSON_TELEMETRY_LABEL "data"
#define DEBUG_JSON_TELEMETRY_UNITS "value"

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
  // extern char bufferTx[];
  // extern char bufferRx[];
  // extern JsonDocument docTx, docRx;

  const __FlashStringHelper* parseType(const msgtype_t& type);

  #ifdef DEBUG_SERIAL

    size_t jsonPrint(JsonDocument& json, Print& out = DEBUG_SERIAL);
    size_t jsonPrintln(JsonDocument& json, Print& out = DEBUG_SERIAL);

    void revision(const uint8_t& version, Print& out = DEBUG_SERIAL);

    // Looks for [key: string]: number
    // void telemetry(JsonObject data, unsigned long timestamp, Print& out = DEBUG_SERIAL);
    // void telemetry(JsonObject data, Print& out = DEBUG_SERIAL);
    template <typename T> void telemetry(unsigned long timestamp, T value, String label, Print& out = DEBUG_SERIAL);
    template <typename T> void telemetry(unsigned long timestamp, T value, Print& out = DEBUG_SERIAL);

    void heartbeat(unsigned long timestamp, Print& out = DEBUG_SERIAL);
    void heartbeat(Print& out = DEBUG_SERIAL);
  
  #else

    size_t jsonPrint(JsonDocument& json, Print& out);
    size_t jsonPrintln(JsonDocument& json, Print& out);

    void revision(const uint8_t& version, Print& out);

    // Looks for [key: string]: number
    // void telemetry(JsonObject data, unsigned long timestamp, Print& out);
    // void telemetry(JsonObject data, Print& out);
    template <typename T> void telemetry(unsigned long timestamp, T value, String label, Print& out);
    template <typename T> void telemetry(unsigned long timestamp, T value, Print& out);

    void heartbeat(unsigned long timestamp, Print& out);
    void heartbeat(Print& out);
  
  #endif

  typedef void (*debugjson_cb_config_t)(JsonObject config);
  typedef void (*debugjson_cb_commands_t)(JsonObject commands);

  #ifdef DEBUG_SERIAL

  void update(Stream& serial = DEBUG_SERIAL, debugjson_cb_commands_t cb_commands = nullptr, debugjson_cb_config_t cb_config = nullptr);
  void fixedUpdate(unsigned long timestamp, Print& out = DEBUG_SERIAL);

  #else

  void update(Stream& serial, debugjson_cb_commands_t cb_commands = nullptr, debugjson_cb_config_t cb_config = nullptr);
  void fixedUpdate(unsigned long timestamp, Print& out);

  #endif

  class StringWriter : public Print {
    private:
      String s = String();
    public:
      StringWriter(const String& s) : StringWriter() { print(s); }
      StringWriter() : Print() {}
      virtual ~StringWriter() {}
      size_t write(uint8_t b) override { s += (char)b; return 1; }
      int availableForWrite() override { return DEBUG_JSON_SIZE_STRINGWRITER - s.length(); } // Safe upper limit
      void flush() override { s = String(); } // Reset
      operator String() const { return s; } // Implicit copy-conversion
  };

  // Appends to ["msg"] and prints on newline
  template <msgtype_t T> class DebugPrint : public Print {
    protected:
      JsonDocument json;
      const msgtype_t type = T;
      Print& out; // Wrapped
    public:
      DebugPrint(Print& s) : out(s) {}
      virtual ~DebugPrint() { out.flush(); }
      size_t write(const uint8_t *buffer, size_t size) override;
      size_t write(uint8_t b) override { return out.write(b); } // Leave as-is (i.e. delineators)
      int availableForWrite() override { return out.availableForWrite(); } // Leave as-is
      void flush() override { out.flush(); } // Leave as-is
      // int available() { return out.available(); } // Leave as-is
      // int read() { return out.read(); } // Leave as-is
      // int peek() { return out.peek(); } // Leave as-is
  };

  // Appends to ["msg"] and prints on newline
  // The right way to serialize JS-ready batch data objects
  // class TelemetryPrint : public DebugPrint<EVENT_TELEMETRY> {
  //   private:
  //     const String label, units;
  //   public:
  //     TelemetryPrint(Print& s, String label, String units = DEBUG_JSON_TELEMETRY_UNITS) : DebugPrint<EVENT_TELEMETRY>(s), label(label), units(units) { units.trim(); json[DEBUG_JSON_TELEMETRY_LABEL][label] = JsonArray(); }
  //     template <typename T> bool setJson(JsonObject& obj, T value);
  //     template <typename T> size_t write(const T* buffer, size_t size) {
  //       bool r = true;
  //       for(size_t i = 0; i < size; i++) {
  //         JsonObject obj = json[DEBUG_JSON_TELEMETRY_LABEL][label].createNestedObject();
  //         r &= obj[units].createNestedObject();
  //         set<T>(buffer[i]);
  //       }
  //       return size;
  //     }
  // };

  // class CommandRouter : public DebugPrint<EVENT_COMMAND>, public Stream {
  //   private:
  //     debugjson_cb_commands_t cb_commands;
  //     debugjson_cb_config_t cb_config;
  //   public:
  //     CommandRouter(Stream& s, debugjson_cb_commands_t cb_commands) : DebugPrint<EVENT_COMMAND>(s), cb_commands(cb_commands), cb_config(cb_config) {}
  //     size_t write(const uint8_t *buffer, size_t size) override;
  // };

};


#ifdef DEBUG_SERIAL
  extern DebugJson::DebugPrint<DebugJson::DEBUG_NONE> DebugJsonBreakpoints; // I.e. no error, breakpoints
  extern DebugJson::DebugPrint<DebugJson::DEBUG_INFO> DebugJsonOut; // I.e. no error, breakpoints
  extern DebugJson::DebugPrint<DebugJson::DEBUG_WARN> DebugJsonWarning; // I.e. software glitch
  extern DebugJson::DebugPrint<DebugJson::DEBUG_ERROR> DebugJsonError; // I.e. hardware failure
  
  #define DEBUG_JSON(...)   DebugJsonOut.println(__VA_ARGS__)
  #define WARNING_JSON(...) DebugJsonWarning.println(__VA_ARGS__)
  #define ERROR_JSON(...)   DebugJsonError.println(__VA_ARGS__)
  #define BP_JSON(...)      DebugJsonBreakpoints.println((String(__FILE__) + ':' + String(__LINE__)) __VA_OPT__(+ ' ' + String(__VA_ARGS__)))
#endif

#include "DebugJson.tpp"

#endif