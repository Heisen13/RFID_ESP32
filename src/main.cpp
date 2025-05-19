#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "PLDTHOMEFIBRgCk8r";
const char* password = "He@rts565656";
const char* apiEndpoint = "http://192.168.1.80:8000/api/attendance/";

struct Tag {
  const char* id;
  const char* name;
};

Tag knownTags[] = {
  {"E63D76D3", "Person 1"},
  {"D3DFC9E", "Person 2"},
  {"5410A8A3", "Person 3"}
};

String getUIDString(MFRC522::Uid uid) {
  String uidStr = "";
  for (byte i = 0; i < uid.size; i++) {
    uidStr += String(uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  return uidStr;
}

String getPersonName(String uid) {
  for (Tag tag : knownTags) {
    if (uid == tag.id) return tag.name;
  }
  return "Unknown";
}

void postToServer(String uid, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiEndpoint);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["tag_id"] = uid;
    doc["name"] = name;

    String jsonStr;
    serializeJson(doc, jsonStr);

    int httpResponseCode = http.POST(jsonStr);
    if (httpResponseCode > 0) {
      Serial.print("✅ Data sent! Response: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("❌ Error sending POST: ");
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("❌ WiFi not connected!");
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("📡 Connecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = getUIDString(rfid.uid);
  String name = getPersonName(uid);

  Serial.print("🔍 Detected Tag: ");
  Serial.println(uid);

  if (name == "Unknown") {
    Serial.println("❌ Tag not recognized.");
  } else {
    Serial.print("✅ Hello, ");
    Serial.println(name);
    postToServer(uid, name);
  }

  rfid.PICC_HaltA();
  delay(3000);
}
