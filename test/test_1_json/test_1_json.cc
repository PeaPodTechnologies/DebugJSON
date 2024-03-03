#include <Arduino.h>
#include <unity.h>
#include <ArduinoJson.h>

#define UNIT_TEST_DELAY 500

JsonDocument json;

void test_json_object(void) {
  json["uint"] = 3;
  TEST_ASSERT_EQUAL(3, json["uint"].as<int>());

  json["int"] = -3;
  TEST_ASSERT_EQUAL(-3, json["int"].as<int>());
  
  json["float"] = 3.14;
  TEST_ASSERT_EQUAL(3.14, json["float"].as<double>());

  json["string"] = "hello";
  TEST_ASSERT_EQUAL_STRING("hello", json["string"].as<const char*>());

  json["bool"] = true;
  TEST_ASSERT_EQUAL(true, json["bool"].as<bool>());
  
  json["null"] = nullptr;
  TEST_ASSERT_EQUAL(nullptr, json["null"].as<const char*>());

  // Reset the JsonObject
  json.clear();
}

void test_json_serialize(void) {
  char* buffer = (char*)malloc(100);
  TEST_ASSERT_NOT_EQUAL(nullptr, buffer);

  // Important to know: edge case for empty object
  serializeJson(json, buffer, 100);
  TEST_ASSERT_EQUAL_STRING("null", buffer);

  json["test"] = 42;
  serializeJson(json, buffer, 100);
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