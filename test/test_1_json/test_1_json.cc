#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>

#define UNIT_TEST_DELAY 500

JsonObject json;

void test_json_object(void) {
  json["uint"] = 3;
  json["int"] = -3;
  json["float"] = 3.14;
  json["string"] = "hello";
  json["bool"] = true;
  json["null"] = nullptr;
  
  TEST_ASSERT_EQUAL(3, json["uint"]);
  TEST_ASSERT_EQUAL(-3, json["int"]);
  TEST_ASSERT_EQUAL(3.14, json["float"]);
  TEST_ASSERT_EQUAL_STRING("hello", json["string"]);
  TEST_ASSERT_EQUAL(true, json["bool"]);
  TEST_ASSERT_EQUAL(nullptr, json["null"]);

  // Reset the JsonObject
  json.clear();
}

void test_json_serialize(void) {
  char* buffer = (char*)malloc(100);
  serializeJson(doc, buffer, 100);
  TEST_ASSERT_EQUAL_STRING("{}", buffer);

  json["test"] = 42;
  serializeJson(doc, buffer, 100);
  TEST_ASSERT_EQUAL_STRING("{\"test\":42}", buffer);

  free(buffer);
}

void setup() {
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_json_object);
  RUN_TEST(test_json_serialize);

  UNITY_END();
}

void loop() {
}