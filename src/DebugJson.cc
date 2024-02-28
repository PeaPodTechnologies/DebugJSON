#pragma once

#include "DebugJson.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>

using namespace DebugJson;

void debug(const msgtype_t& level, const char *msg) {
  JsonDocument doc;
  doc["level"] = level;
  if(msg != nullptr) doc["msg"] = msg;
  serializeJson(doc, DEBUG_SERIAL);
}

template <typename T> void debugState(const msgtype_t& level, const T& state, const char* label, const char *msg) {
  JsonDocument doc;
  doc["level"] = level;
  doc[label] = state;
  if(msg != nullptr) doc["msg"] = msg;
  serializeJson(doc, DEBUG_SERIAL);
}