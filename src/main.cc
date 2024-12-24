#include "DebugJson.h"

using namespace DebugJson;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  DebugStream<DEBUG_INFO> out = DebugStream<DEBUG_INFO>(Serial);
  out.println(F("Hello, world!"));
  delay(2000);
}

JsonDocument json;

bool r = false;
void loop() {
  JsonObject obj = json.to<JsonObject>();
  obj["bool"] = r;
  obj["int"] = rand();
  DebugJson::telemetry(millis(), obj, Serial);
  r = !r;
  delay(2000);
}