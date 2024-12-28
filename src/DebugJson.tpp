#ifndef DEBUG_JSON_H_
#error "This file should not be included directly, include DebugJson.h instead"
#endif

#ifdef DEBUG_JSON_H_
#ifndef DEBUG_JSON_T_
#define DEBUG_JSON_T_

template <typename T> void DebugJson::telemetry(unsigned long timestamp, T value, String key, Print& out) {
  JsonDocument docTx;
  // if(docTx["data"][key].isNull() || docTx["data"]["count"][key].isNull()) {
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["data"][key] = value;
  docTx["timestamp"] = timestamp;
  jsonPrintln(docTx, out);
}

template <typename T> void DebugJson::telemetry(unsigned long timestamp, T value, Print& out) {
  JsonDocument docTx;
  // if(docTx["data"][key].isNull() || docTx["data"]["count"][key].isNull()) {
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["data"] = value;
  docTx["timestamp"] = timestamp;
  jsonPrintln(docTx, out);
}

template <DebugJson::msgtype_t T, char D> size_t DebugJson::DebugPrint<T, D>::write(const uint8_t *buffer, size_t size) {
  #ifdef DEBUG_JSON_LEVEL_MIN
  if(T < DEBUG_JSON_LEVEL_MIN) {
    return 0;
  }
  #endif
  bool print = false;
  if(size == 0) return 0;
  size_t slen = this->msg.length();
  if(slen > 0) this->msg += DEBUG_JSON_MSG_SEP;
  this->msg += (const char*)buffer;

  if((char)buffer[size - 1] == D) {
    this->msg.trim(); // Remove trailing whitespace
    // This is a println() call; Send the document
    JsonDocument json;
    json["type"] = DebugJson::parseType(this->type);
    json["timestamp"] = millis();
    json["msg"] = this->msg;
    return DebugJson::jsonPrintln(json, this->out); // Also clears the document
    this->msg = String(); // Reset
  } else {
    // return s.length() - slen;
    return size;
  }
}

#endif
#endif