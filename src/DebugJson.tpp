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
    docTx.clear();
  // }
  // JsonDocument docTx;
  docTx["type"] = F("telemetry");
  docTx["data"] = value;
  docTx["timestamp"] = timestamp;
  // docTx.shrinkToFit();
  // jsonPrint(timestamp, out);
  jsonPrintln(out);
}

template <DebugJson::msgtype_t T> size_t DebugJson::DebugStream<T>::write(const uint8_t *buffer, size_t size) {
  #ifdef DEBUG_JSON_LEVEL_MIN
  if(T < DEBUG_JSON_LEVEL_MIN) {
    return 0;
  }
  #endif
  if(size <= 2 && buffer[size - 1] == '\n') {
    // This is a println() call - treat it as a normal print
    return out.write(buffer, size);
  }
  // debug(type, (const char*)buffer, size, out);
  DebugJson::docTx.clear();
  DebugJson::docTx["type"] = DebugJson::parseType(this->type);
  DebugJson::docTx["timestamp"] = millis();
  // size = min(size, strlen((const char*)buffer));
  // String s; for(size_t i = 0; i < size; i++) { s += (char)buffer[i]; }
  DebugJson::docTx["msg"] = (const char*)buffer;
  // docTx.shrinkToFit();

  return DebugJson::jsonPrint(this->out); // NOT println; this will be called with "\r\n" by the Stream class
}

#endif
#endif