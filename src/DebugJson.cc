#include "DebugJson.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>

unsigned long lastTx = 0;
unsigned long lastRx = 0;

unsigned posBufferRx = 0;
bool delimit = false; // TODO move to a class
char DebugJson::bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' };
char DebugJson::bufferRx[DEBUG_JSON_SIZE_DOC] = { '\0' };

// char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' };
// char bufferRx[DEBUG_JSON_SIZE_DOC] = { '\0' };
JsonDocument DebugJson::docTx;
JsonDocument DebugJson::docRx;

#ifdef DEBUG_SERIAL
  DebugJson::DebugStream<DebugJson::DEBUG_INFO> DebugJsonOut(DEBUG_SERIAL); // I.e. no error, breakpoints
  DebugJson::DebugStream<DebugJson::DEBUG_WARN> DebugJsonWarning(DEBUG_SERIAL); // I.e. software glitch
  DebugJson::DebugStream<DebugJson::DEBUG_ERROR> DebugJsonError(DEBUG_SERIAL); // I.e. hardware failure
#endif

// NOTE THE NAMING CONVENTIONS USED HERE
const __FlashStringHelper* DebugJson::parseType(const DebugJson::msgtype_t& type) {
  switch(type) {
    case DebugJson::DEBUG_INFO: return F("debug");
    case DebugJson::DEBUG_WARN: return F("error");
    case DebugJson::DEBUG_ERROR: return F("fatal");
    case DebugJson::EVENT_TELEMETRY: return F("event");
    case DebugJson::EVENT_COMMAND: return F("command");
    case DebugJson::EVENT_CONFIGURATION: return F("config");
    case DebugJson::EVENT_REVISION: return F("revision");
    default: return F("info");
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
  lastTx = millis();
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
  lastTx = millis();
}

void printSerial(const String& buffer, Stream& out) {
  printSerial(buffer.c_str(), buffer.length(), out);
}

void DebugJson::update(Stream& serial, debugjson_cb_commands_t cb_commands, debugjson_cb_config_t cb_config) {
  // Read to async global buffer
  while(serial.available() > 0) {
    char c = serial.read();
    lastRx = millis();
    if(c == '\n') {
      delimit = true;
      bufferRx[posBufferRx] = '\0';
      // serial.println(bufferRx);
      break;
    } else {
      bufferRx[posBufferRx] = c;
      posBufferRx++;
    }
  }
  if(delimit){ // bufferRx[posBufferRx] == '\0'
    if(posBufferRx == 0) {
      delimit = false;
      return;
    }
    docRx.clear();
    DeserializationError error = deserializeJson(docRx, bufferRx, posBufferRx);

    // unsigned i;
    // for(i = 0; i < DEBUG_JSON_SIZE_DOC && bufferRx[i + posBufferRx] != '\0'; i++) {
    //   bufferRx[i] = bufferRx[i + posBufferRx];
    // }

    // bufferRx[i] = '\0';
    // posBufferRx = 0;

    // serial.println(docRx["type"].as<String>());

    if (error || docRx.isNull()) {
      // DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: Failed to parse JSON (")) + error.c_str() + ')');
      #ifdef DEBUG_SERIAL
        String s = F("DJ: Failed to parse JSON (");
        s += error.c_str();
        s += ')';
        DebugJsonWarning.println(s);
      #endif
    } else {
      delimit = false;
      posBufferRx = 0;
      if(docRx["type"].isNull() || !docRx["type"].is<String>() || docRx["type"].as<String>().length() == 0) {
        // DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: Invalid JSON (ENOENT 'type')")));
        #ifdef DEBUG_SERIAL
          DebugJsonWarning.println(F("DJ: Invalid JSON (ENOENT 'type')"));
        #endif
      } else {
        String type = docRx["type"];
        
        if(type == String("command")) {
          // TODO: Command validation
          JsonObject data = docRx["data"];
          if(cb_commands != nullptr) cb_commands(data);
          // else DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback 'Commands'")));
          #ifdef DEBUG_SERIAL
            DebugJsonWarning.println(F("DJ: No Callback 'Commands'"));
          #endif
        } else if (type == String("config")) {
          // TODO: Config validation
          JsonObject data = docRx["data"];
          if(cb_config != nullptr) cb_config(data);
          // else DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback 'Configuration'")));
          #ifdef DEBUG_SERIAL
            DebugJsonWarning.println(F("DJ: No Callback 'Configuration'"));
          #endif
        } else {
          // DebugJson::debug(DebugJson::DEBUG_WARN, String(F("DJ: No Callback '")) + String(type) + '\'');
          #ifdef DEBUG_SERIAL
            String msg = F("DJ: No Callback '");
            msg += type;
            msg += '\'';
            DebugJsonWarning.println(msg);
          #endif
        }
      }
    }
  }
}

void DebugJson::fixedUpdate(unsigned long timestamp, Stream& serial) {
  if((timestamp - lastTx) > DEBUG_JSON_HEARTBEAT) {
    DebugJson::heartbeat(timestamp, serial);
  }
}

size_t DebugJson::jsonPrintln(Stream& out) {
  // if(docTx.isNull() || docTx.overflowed() || docTx["type"].isNull() || docTx["type"].as<String>().length() == 0) {
  //   // out.println('~');
  //   docTx.clear();
  //   // docTx.shrinkToFit();
  //   return; // Misfire
  // }

  // char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' }; // LOCAL BUFFER BREAKS THE FUNCTION AND THE WHOLE PROGRAM WITH IT DO NOT UNCOMMENT THIS LINE
  memset(bufferTx, '\0', DEBUG_JSON_SIZE_DOC);
  size_t n = serializeJson(docTx, bufferTx);
  docTx.clear();
  if(n == 0) {
    return 0; // Misfire
  }

  // Append newline and null-terminate
  bufferTx[n] = '\n';

  // Print in chunks based on Serial.availableForWrite()
  printSerial(bufferTx, n+1, out);

  docTx.clear();
  return n+1;
}

size_t DebugJson::jsonPrint(Stream& out) {
  // if(docTx.isNull() || docTx.overflowed() || docTx["type"].isNull() || docTx["type"].as<String>().length() == 0) {
  //   // out.println('~');
  //   docTx.clear();
  //   // docTx.shrinkToFit();
  //   return; // Misfire
  // }

  // char bufferTx[DEBUG_JSON_SIZE_DOC] = { '\0' }; // LOCAL BUFFER BREAKS THE FUNCTION AND THE WHOLE PROGRAM WITH IT DO NOT UNCOMMENT THIS LINE
  memset(bufferTx, '\0', DEBUG_JSON_SIZE_DOC);
  size_t n = serializeJson(docTx, bufferTx);
  docTx.clear();
  if(n == 0) {
    return 0; // Misfire
  }

  // Print in chunks based on Serial.availableForWrite()
  printSerial(bufferTx, n, out);

  docTx.clear();
  return n;
}

void DebugJson::revision(const uint8_t& version, Stream& out) {
  if(!docTx.isNull()) {
    jsonPrintln(out);
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = parseType(EVENT_REVISION);
  docTx["data"] = version;
  // docTx.shrinkToFit();
  jsonPrintln(out);
}

// Looks for [key: string]: number
void DebugJson::telemetry(unsigned long timestamp, JsonObject data, Stream& out) {
  if(!docTx.isNull()) {
    // if(docTx["timestamp"].isNull()) jsonPrint(timestamp, out);
    // else{ 
      jsonPrint(out);
    // }
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["data"] = data;
  docTx["timestamp"] = timestamp;
  // docTx.shrinkToFit();
  // jsonPrint(timestamp, out);
  jsonPrintln(out);
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

template <typename T> void DebugJson::telemetry(unsigned long timestamp, T value, Stream& out) {
  if(!docTx.isNull()) {
    // if(docTx["timestamp"].isNull()) jsonPrint(timestamp, out);
    // else {
      jsonPrint(out);
    // }
    docTx.clear();
  }
  // JsonDocument docTx;
  docTx["type"] = F("telemetry");
  docTx["data"] = value;
  docTx["timestamp"] = timestamp;
  // docTx.shrinkToFit();
  // jsonPrint(timestamp, out);
  jsonPrintln(out);
}

void DebugJson::heartbeat(unsigned long timestamp, Stream& out) {
  if(!docTx.isNull()) {
    // if(docTx["timestamp"].isNull()) jsonPrint(timestamp, out);
    // else {
      jsonPrint(out);
    // }
  } else {
    // Empty document
    // JsonDocument docTx;
    docTx["type"] = F("heartbeat");
    docTx["timestamp"] = timestamp;
    jsonPrintln(out);
  }
}
void DebugJson::heartbeat(Stream& out) { heartbeat(millis(), out); }