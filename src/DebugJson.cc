#include "DebugJson.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>

unsigned long lastTxMicros = 0;
// unsigned long lastTxMicros = 0; // For frame refresh
// unsigned long lastRx = 0;

// unsigned numBufferRx = 0;
char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' };
JsonDocument docTx;

// NOTE THE NAMING CONVENTIONS USED HERE
const __FlashStringHelper* parseType(const DebugJson::msgtype_t& type) {
  switch(type) {
    case DebugJson::DEBUG_NONE: return F("info");
    case DebugJson::DEBUG_INFO: return F("debug");
    case DebugJson::DEBUG_WARN: return F("error");
    case DebugJson::DEBUG_ERROR: return F("fatal");
    case DebugJson::EVENT_TELEMETRY: return F("event");
    case DebugJson::EVENT_COMMAND: return F("command");
    case DebugJson::EVENT_CONFIGURATION: return F("config");
  }
  return nullptr;
}

void printSerial(const char* buffer, size_t len, Stream& out) {
  size_t pos = 0;
  while(pos < len) {
    size_t chunk = min(len - pos, (size_t)out.availableForWrite());
    if(chunk == 0) {
      out.flush();
      continue;
    }
    out.write(buffer + pos, chunk);
    pos += chunk;
  }
}

void printSerial(const __FlashStringHelper* buffer, size_t len, Stream& out) {
  size_t pos = 0;
  while(pos < len) {
    size_t chunk = min(len - pos, (size_t)out.availableForWrite());
    if(chunk == 0) {
      out.flush();
      continue;
    }
    char c = pgm_read_byte_near((unsigned int)buffer + pos);
    out.write(c);
    pos ++;
  }
}

void printSerial(const String& buffer, Stream& out) {
  printSerial(buffer.c_str(), buffer.length(), out);
}

// void DebugJson::update(debugjson_cb_commands_t cb_commands, debugjson_cb_config_t cb_config) {
//   bool delimit = false;
//   if(Serial.available() > 0) {
//     char bufferRx[DEBUG_JSON_SIZE_DOC] = { '\0' };
//     // memset(bufferRx, '\0', DEBUG_JSON_SIZE_DOC);
//     while(Serial.available() > 0) {
//       lastRx = millis();
//       char c = Serial.read();
//       if(c == '\n') {
//         delimit = true;
//         bufferRx[numBufferRx] = '\0';
//         break;
//       }
//       bufferRx[numBufferRx] = c;
//       numBufferRx++;
//     }
//     if(delimit){
//       DeserializationError error = deserializeJson(docRx, bufferRx);
//       if (error || docRx.isNull() || docRx["type"].isNull()) {
//         DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: Failed to parse JSON (")) + error.c_str() + ") '" + String(bufferRx) + '\'');
//       } else {
//         if(docRx["type"].isNull()) {
//           DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: Invalid JSON (ENOENT 'type') '")) + String(bufferRx) + "'");
//         } else {
//           String type = docRx["type"];
//           if(type == String("command")) {
//             // TODO: Command validation
//             if(cb_commands != nullptr) cb_commands(docRx);
//             else DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback: 'Commands'")));
//           } else if (type == String("config")) {
//             // TODO: Config validation
//             if(cb_config != nullptr) cb_config(docRx);
//             else DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback 'Configuration'")));
//           } else {
//             DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback: '")) + String(type) + '\'');
//           }
//         }
//       }
//     }
//   }
// }

// void DebugJson::fixedUpdate(unsigned long timestamp) {
//   if((timestamp - lastTx) > DEBUG_JSON_HEARTBEAT) {
//     DebugJson::heartbeat(timestamp);
//   }
// }

void DebugJson::jsonPrint(const __FlashStringHelper* msg, size_t len, Stream& out) {
  if(docTx.isNull() || docTx.overflowed() || !docTx.containsKey("type") || docTx["type"].isNull() || docTx["type"].as<String>().length() == 0) {
    // out.println('~');
    docTx.clear();
    // docTx.shrinkToFit();
    return; // Misfire
  }

  if(msg == nullptr || len == 0 || docTx.containsKey("msg")) {
    jsonPrint(out);
    return;
  }

  // char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' };
  memset(bufferTx, '\0', DEBUG_JSON_SIZE_DOC);
  size_t n = serializeJson(docTx, bufferTx);
  docTx.clear();
  if(n == 0) {
    return; // Misfire
  }

  // Find last closing brace
  char* p = bufferTx + n;
  while(p > bufferTx && *p != '}') p--;
  if(p > bufferTx) {
    *p = ','; // Replace with comma, retain null-termination
    // *(p+1) = '\0'; // Null-terminate (just in case)

    printSerial(bufferTx, p - bufferTx + 1, out);
    printSerial(F("\"msg\":\""), 7, out);
    printSerial(msg, len, out);
    printSerial(F("\"}\n"), 3, out);
  } else {
    printSerial(bufferTx, n, out);
  }
}
void DebugJson::jsonPrint(const char* msg, size_t len, Stream& out) {
  if(docTx.isNull() || docTx.overflowed() || !docTx.containsKey("type") || docTx["type"].isNull() || docTx["type"].as<String>().length() == 0) {
    // out.println('~');
    docTx.clear();
    // docTx.shrinkToFit();
    return; // Misfire
  }

  if(msg == nullptr || len == 0 || docTx.containsKey("msg")) {
    jsonPrint(out);
    return;
  }

  // char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' };
  memset(bufferTx, '\0', DEBUG_JSON_SIZE_DOC);
  size_t n = serializeJson(docTx, bufferTx);
  docTx.clear();
  if(n == 0) {
    return; // Misfire
  }

  // Find last closing brace
  char* p = bufferTx + n;
  while(p > bufferTx && *p != '}') p--;
  if(p > bufferTx) {
    *p = ','; // Replace with comma, retain null-termination
    *(p+1) = '\0'; // Null-terminate (just in case)

    printSerial(bufferTx, p - bufferTx + 1, out);
    printSerial(F("\"msg\":\""), 7, out);
    printSerial(msg, len, out);
    printSerial(F("\"}\n"), 3, out);
  } else {
    printSerial(bufferTx, n, out);
  }
}

void DebugJson::jsonPrint(const String& msg, Stream& out) {
  jsonPrint(msg.c_str(), msg.length(), out);
}

void DebugJson::jsonPrint(unsigned long timestamp, const String& msg, Stream& out) { 
  docTx["timestamp"] = timestamp;
  jsonPrint(msg, out);
}

void DebugJson::jsonPrint(unsigned long timestamp, const __FlashStringHelper* msg, size_t len, Stream& out) { 
  docTx["timestamp"] = timestamp;
  jsonPrint(msg, len, out);
}

void DebugJson::jsonPrint(unsigned long timestamp, const char* msg, size_t len, Stream& out) { 
  docTx["timestamp"] = timestamp;
  jsonPrint(msg, len, out);
}

void DebugJson::jsonPrint(unsigned long timestamp, Stream& out) {
  docTx["timestamp"] = timestamp;
  jsonPrint(out);
}

void DebugJson::jsonPrint(Stream& out) {
  if(docTx.isNull() || docTx.overflowed() || !docTx.containsKey("type") || docTx["type"].isNull() || docTx["type"].as<String>().length() == 0) {
    // out.println('~');
    docTx.clear();
    // docTx.shrinkToFit();
    return; // Misfire
  }

  // char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' }; // LOCAL BUFFER BREAKS THE FUNCTION AND THE WHOLE PROGRAM WITH IT DO NOT UNCOMMENT THIS LINE
  memset(bufferTx, '\0', DEBUG_JSON_SIZE_DOC);
  size_t n = serializeJson(docTx, bufferTx);
  docTx.clear();
  if(n == 0) {
    return; // Misfire
  }

  // Append newline and null-terminate
  bufferTx[n] = '\n';

  // Print in chunks based on Serial.availableForWrite()
  printSerial(bufferTx, n, out);

  docTx.clear();
}

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  
    ? (int)&__heap_start : (int) __brkval);  
}

void DebugJson::debug(const String& msg) { debug(DEBUG_NONE, msg); }
void DebugJson::debug(const char* msg, size_t len) { debug(DEBUG_NONE, msg, len); }
void DebugJson::debug(const __FlashStringHelper* msg, size_t len) { debug(DEBUG_NONE, msg, len); }

void DebugJson::debug(const DebugJson::msgtype_t& type, const String& msg) { debug(type, msg.c_str(), msg.length()); }

void DebugJson::debug(const DebugJson::msgtype_t& type, const __FlashStringHelper* msg, size_t len) {
  if(type < DEBUG_JSON_LEVEL_MIN) return;
  // JsonDocument docTx;
  docTx["type"] = parseType(type);
  docTx["timestamp"] = millis();
  // if(msg.length() > 0) docTx["msg"] = msg;
  #ifdef DEBUG_JSON_SRAM
    docTx["data"]["sram"] = freeRam();
  #endif
  docTx.shrinkToFit();
  jsonPrint(msg, len);
}
void DebugJson::debug(const DebugJson::msgtype_t& type, const char* msg, size_t len) {
  if(type < DEBUG_JSON_LEVEL_MIN) return;
  if(!docTx.isNull()) {
    jsonPrint();
  }
  docTx.clear();
  // JsonDocument docTx;
  docTx["type"] = parseType(type);
  docTx["timestamp"] = millis();
  // if(msg.length() > 0) docTx["msg"] = msg;
  #ifdef DEBUG_JSON_SRAM
    docTx["data"]["sram"] = freeRam();
  #endif
  docTx.shrinkToFit();
  jsonPrint(msg, len);
  // Serial.print(F("{\"type\":\"")); Serial.print(parseType(type)); Serial.print(F("\",\"msg\":\"")); Serial.print(msg); Serial.println("\"}");
}

// template <typename T> void DebugJson::debugState(const msgtype_t& level, const T& state, const char* label, const char *msg) {
//   JsonDocument doc;
//   doc["level"] = level;
//   doc[label] = state;
//   if(msg != nullptr) doc["msg"] = msg;
//   serializeJson(doc, buffer);
// }

void DebugJson::revision(const uint8_t& version) {
  if(!docTx.isNull()) {
    jsonPrint();
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = parseType(EVENT_REVISION);
  docTx["data"] = version;
  docTx.shrinkToFit();
  jsonPrint();
}

// Looks for [key: string]: number
void DebugJson::telemetry(unsigned long timestamp, JsonObject& data) {
  if(!docTx.isNull()) {
    if(docTx["timestamp"].isNull()) jsonPrint(timestamp);
    else jsonPrint();
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["data"] = data;
  docTx.shrinkToFit();
  jsonPrint(timestamp);
}

// template <typename T> unsigned DebugJson::telemetry(unsigned long timestamp, String key, T value, bool send) {
//   if(docTx["type"] != String("telemetry")) {
//     if(docTx["timestamp"].isNull()) jsonPrint(timestamp, docTx);
//     else jsonPrint(docTx);

//     docTx["type"] = "telemetry";
//   }
//   if(docTx["data"][key].isNull() || docTx["data"]["count"][key].isNull()) {
//     docTx["data"][key] = JsonArray();
//     docTx["data"]["timestamps"][key] = JsonArray();
//     docTx["data"]["count"][key] = 0;
//   }
//   unsigned count = docTx["data"]["count"][key];
//   docTx["data"][key][count] = value;
//   docTx["data"]["timestamps"][key][count] = docTx["data"]["timestamps"].isNull() ? JsonArray({ timestamp }) : timestamp; // Add timestamp
//   count++;
//   docTx["data"]["count"][key] = count;
//   if(send) jsonPrint(docTx);
//   return count;
// }

template <typename T> void DebugJson::telemetry(unsigned long timestamp, T value) {
  if(!docTx.isNull()) {
    if(docTx["timestamp"].isNull()) jsonPrint(timestamp);
    else jsonPrint();
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = F("telemetry");
  docTx["data"] = value;
  docTx.shrinkToFit();
  jsonPrint(timestamp);
}

void DebugJson::heartbeat(unsigned long timestamp) {
  if(!docTx.isNull()) {
    if(docTx["timestamp"].isNull()) jsonPrint(timestamp);
    else jsonPrint();
  } else {
    // Empty document
    // JsonDocument docTx;
    docTx["type"] = F("heartbeat");
    jsonPrint(timestamp);
  }
}