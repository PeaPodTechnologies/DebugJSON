#ifndef DEBUG_JSON_H_
#error "This file should not be included directly, include DebugJson.h instead"
#endif

#ifdef DEBUG_JSON_H_
#ifndef DEBUG_JSON_T_
#define DEBUG_JSON_T_

template <typename S, const char* T> void DebugJson::telemetry(unsigned long timestamp, S value, String key, Print& out) {
  JsonDocument docTx;
  // if(docTx["data"][key].isNull() || docTx["data"]["count"][key].isNull()) {
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["timestamp"] = timestamp;
  if(T != nullptr) {
    docTx["t"] = T;
  }
  docTx["data"][key] = value;
  jsonPrintln(docTx, out);
}

template <typename S, const char* T> void DebugJson::telemetry(unsigned long timestamp, S value, Print& out) {
  JsonDocument docTx;
  // if(docTx["data"][key].isNull() || docTx["data"]["count"][key].isNull()) {
  docTx["type"] = parseType(EVENT_TELEMETRY);
  docTx["timestamp"] = timestamp;
  if(T != nullptr) {
    docTx["t"] = T;
  }
  docTx["data"] = value;
  jsonPrintln(docTx, out);
}

// const char* printWhitespace(char c) {
//   switch(c) {
//     case '\n': return "\\n";
//     case '\r': return "\\r";
//     case '\t': return "\\t";
//     case '\v': return "\\v";
//     case '\f': return "\\f";
//     case '\b': return "\\b";
//     case '\a': return "\\a";
//     case '\0': return "\\0";
//     default: return "";
//   }
// }

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

  if(this->msg.indexOf(D) != -1) {
    this->msg.trim(); // Remove trailing whitespace
    // This is a println() call; Send the document
    JsonDocument json;
    json["type"] = DebugJson::parseType(this->type);
    json["timestamp"] = millis();
    json["msg"] = this->msg;
    size_t r = DebugJson::jsonPrintln(json, this->out, D); // Also clears the document
    
    this->msg.clear(); // Clear string
    return r;
  } else {
    // return s.length() - slen;
    return size;
  }
}

#endif
#endif