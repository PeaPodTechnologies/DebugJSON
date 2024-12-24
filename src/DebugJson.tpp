#ifndef DEBUG_JSON_H_
#error "This file should not be included directly, include DebugJson.h instead"
#endif

#ifdef DEBUG_JSON_H_
#ifndef DEBUG_JSON_T_
#define DEBUG_JSON_T_

template <DebugJson::msgtype_t T> size_t DebugJson::DebugStream<T>::write(const uint8_t *buffer, size_t size) {
  if(size <= 2 && buffer[size - 1] == '\n') {
    // This is a println() call - treat it as a normal print
    return out.write(buffer, size);
  }
  // debug(type, (const char*)buffer, size, out);
  DebugJson::docTx.clear();
  DebugJson::docTx["type"] = DebugJson::parseType(this->type);
  DebugJson::docTx["timestamp"] = millis();
  size = min(size, strlen((const char*)buffer));
  String s; for(size_t i = 0; i < size; i++) { s += (char)buffer[i]; }
  DebugJson::docTx["msg"] = s;
  // docTx.shrinkToFit();

  return DebugJson::jsonPrint(this->out);
}

#endif
#endif