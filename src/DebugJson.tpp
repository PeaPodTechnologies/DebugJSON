#ifndef DEBUG_JSON_H_
#error "This file should not be included directly, include DebugJson.h instead"
#endif

#ifdef DEBUG_JSON_H_
#ifndef DEBUG_JSON_T_
#define DEBUG_JSON_T_

template <typename T> void DebugJson::telemetry(unsigned long timestamp, T value, Stream& out) {
  // if(!docTx.isNull()) {
    // if(docTx["timestamp"].isNull()) jsonPrint(timestamp, out);
    // else {
      // jsonPrintln(out);
    // }
  JsonDocument docTx;
  // }
  // JsonDocument docTx;
  docTx["type"] = F("telemetry");
  docTx["data"] = value;
  docTx["timestamp"] = timestamp;
  // docTx.shrinkToFit();
  // jsonPrint(timestamp, out);
  jsonPrintln(docTx, out);
}

template <DebugJson::msgtype_t T> size_t DebugJson::DebugPrint<T>::write(const uint8_t *buffer, size_t size) {
  #ifdef DEBUG_JSON_LEVEL_MIN
  if(T < DEBUG_JSON_LEVEL_MIN) {
    return 0;
  }
  #endif
  if(size > 1 && buffer[size - 1] == '\n') {
    // This is a println() call; Send the document
    return DebugJson::jsonPrintln(this->json, this->out);
  }
  // debug(type, (const char*)buffer, size, out);
  // DebugJson::docTx.clear();

  // OVERWRITE:
  this->json["type"] = DebugJson::parseType(this->type);
  this->json["timestamp"] = millis();

  // size = min(size, strlen((const char*)buffer));
  // String s; for(size_t i = 0; i < size; i++) { s += (char)buffer[i]; }

  // APPEND:
  String s = (this->json["msg"].isNull() || this->json["msg"].as<String>().length() == 0) ? "" : (this->json["msg"].as<String>() + DEBUG_JSON_MSG_SEP);
  this->json["msg"] = s + String((const char*)buffer);

  // docTx.shrinkToFit();

  return size; // NOT println; don't send anything
}

#endif
#endif