#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ESP_DoubleResetDetector.h>

#include "config.h"
#include "html.h"

// ----------------- Globals -----------------
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

WebServer server(80);
AsyncMqttClient mqtt;
Preferences prefs;

bool companyMode = false;     // set in setup()
bool forceCompany = false;    // one-shot flag set by double-tap

String sta_ssid;              // active STA credentials (loaded from NVS or defaults)
String sta_password;

int  TOUCH_THRESHOLD = TOUCH_THR_INIT;
int  touchCount   = 0;
bool wasTouched   = false;
bool staReady     = false;
bool mqttEnabled  = true;     // toggle via API (customer mode)

char ap_ssid_company[40];     // "ESP32_TouchCounter-SVC-XXXX"

// ================== Helpers ==================
void loadWifiCreds() {
  prefs.begin("wifi", true);
  sta_ssid      = prefs.getString("ssid", DEFAULT_STA_SSID);
  sta_password  = prefs.getString("password", DEFAULT_STA_PASSWORD);
  prefs.end();
}
void saveWifiCreds(const String& ssid, const String& pass) {
  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("password", pass);
  prefs.end();
}
String macSuffix4() {
  uint32_t mac4 = (uint32_t)(ESP.getEfuseMac() & 0xFFFF);
  char buf[6];
  sprintf(buf, "%04X", mac4);
  return String(buf);
}

// --- Service double-tap detector on GPIO14/T6 ---
struct TapState {
  bool wasTouched = false;
  uint8_t taps = 0;
  uint32_t firstTapAt = 0;
} svcTap;

bool svcDoubleTapDetected() {
  int v = touchRead(SVC_TOUCH_PIN);
  bool touched = (v < SVC_TOUCH_THR);

  if (touched && !svcTap.wasTouched) {
    uint32_t now = millis();
    if (svcTap.taps == 0) {
      svcTap.taps = 1;
      svcTap.firstTapAt = now;
    } else {
      if (now - svcTap.firstTapAt <= SVC_DOUBLE_TAP_WINDOW_MS) {
        svcTap.taps = 0;
        svcTap.wasTouched = touched;
        return true;
      } else {
        svcTap.taps = 1;
        svcTap.firstTapAt = now;
      }
    }
  }
  if (svcTap.taps == 1 && (millis() - svcTap.firstTapAt > SVC_DOUBLE_TAP_WINDOW_MS)) {
    svcTap.taps = 0;
  }
  svcTap.wasTouched = touched;
  return false;
}

// ================== JSON STATE (customer mode) ==================
String jsonStateCustomer() {
  StaticJsonDocument<512> doc;
  bool staConn = (WiFi.status() == WL_CONNECTED);
  doc["count"]          = touchCount;
  doc["threshold"]      = TOUCH_THRESHOLD;
  doc["sta_connected"]  = staConn;
  doc["sta_ip"]         = staConn ? WiFi.localIP().toString() : "";
  doc["sta_ssid"]       = staConn ? WiFi.SSID() : "";
  doc["ap_ip"]          = WiFi.softAPIP().toString();
  doc["rssi"]           = staConn ? WiFi.RSSI() : 0;
  doc["mqtt_enabled"]   = mqttEnabled;
  doc["mqtt_connected"] = mqtt.connected();
  doc["qos"]            = 2;
  doc["uptime_s"]       = (millis()/1000);
  String out; serializeJson(doc, out); return out;
}

// ================== HTTP HANDLERS ==================
void handleRootCustomer()     { server.send(200, "text/html", buildCustomerHtml()); }
void handleRootCompany()      { server.send(200, "text/html", buildCompanyHtml(ap_ssid_company)); }
void handleApiState()         { server.send(200, "application/json", jsonStateCustomer()); }

void handleApiReset() {
  touchCount = 0;
  if (mqttEnabled && mqtt.connected()) {
    String p = String(touchCount);
    mqtt.publish(MQTT_TOPIC_COUNT, 2, true, p.c_str());
  }
  server.send(200, "application/json", "{\"ok\":true}");
}
void handleApiMqtt() {
  if (!server.hasArg("plain")) { server.send(400,"text/plain","no body"); return; }
  StaticJsonDocument<64> d;
  if (deserializeJson(d, server.arg("plain"))) { server.send(400,"text/plain","bad json"); return; }
  mqttEnabled = d["enabled"] | true;
  server.send(200,"application/json","{\"ok\":true}");
}
void handleApiThreshold() {
  String body = server.arg("plain");
  int newThr = TOUCH_THRESHOLD;
  if (body.length()) {
    StaticJsonDocument<128> doc;
    DeserializationError e = deserializeJson(doc, body);
    if (!e && doc.containsKey("value")) newThr = doc["value"].as<int>();
  } else if (server.hasArg("value")) {
    newThr = server.arg("value").toInt();
  }
  newThr = constrain(newThr, 1, 255);
  TOUCH_THRESHOLD = newThr;
  String resp = String("{\"ok\":true,\"threshold\":") + TOUCH_THRESHOLD + "}";
  server.send(200, "application/json", resp);
}
void handleApiWifi() {
  if (!server.hasArg("plain")) { server.send(400,"text/plain","no body"); return; }
  StaticJsonDocument<192> doc;
  if (deserializeJson(doc, server.arg("plain"))) { server.send(400,"text/plain","bad json"); return; }
  String ssid = doc["ssid"] | "";
  String pass = doc["password"] | "";
  if (ssid.isEmpty()) { server.send(400,"text/plain","SSID required"); return; }

  saveWifiCreds(ssid, pass);
  sta_ssid = ssid; sta_password = pass;

  Serial.printf("WiFi connect requested to SSID: %s\n", ssid.c_str());
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 12000) { delay(300); Serial.print("."); }
  staReady = (WiFi.status() == WL_CONNECTED);
  server.send(200,"application/json", staReady ? "{\"ok\":true,\"connected\":true}" : "{\"ok\":true,\"connected\":false}");
}
void handleApiDisconnect() {
  WiFi.disconnect(true, false);   // keep NVS creds, drop connection
  staReady = false;
  server.send(200,"application/json","{\"disconnected\":true}");
}
void handleSvcPing() { server.send(200, "application/json", String("{\"ok\":true,\"ts\":") + millis() + "}"); }

// ================== WIFI STARTERS ==================
void startAp(const char* ssid, const char* pass) {
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(ssid, pass);
  Serial.printf("AP %s : %s\n", ssid, ok ? "OK" : "FAILED");
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
}
void startApStaCustomer() {
  WiFi.mode(WIFI_AP_STA);
  bool ok = WiFi.softAP(AP_SSID_CUSTOMER, AP_PASS_CUSTOMER);
  Serial.printf("AP %s : %s\n", AP_SSID_CUSTOMER, ok ? "OK" : "FAILED");
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());

  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.printf("Connecting STA to %s", sta_ssid.c_str());
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) { delay(300); Serial.print("."); }
  if (WiFi.status() == WL_CONNECTED) { staReady = true; Serial.printf("\nSTA IP: %s\n", WiFi.localIP().toString().c_str()); }
  else { staReady = false; Serial.println("\nSTA connect timed out (AP remains available)."); }
}

// ================== MQTT (customer mode only) ==================
void mqttConnect() {
  if (!staReady) return;
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  String cid = "ESP32TouchClient-"; cid += String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFF), HEX);
  mqtt.setClientId(cid.c_str());
  mqtt.setWill(MQTT_TOPIC_STAT, 2 /*QoS*/, true /*retain*/, MQTT_WILL_PAY);  // LWT QoS2 retained
  mqtt.connect();
}
void onMqttConnect(bool) {
  Serial.println("MQTT connected (QoS2).");
  mqtt.publish(MQTT_TOPIC_STAT, 2, true, MQTT_BIRTH_PAY);                     // Birth retained
  String p = String(touchCount); mqtt.publish(MQTT_TOPIC_COUNT, 2, true, p.c_str()); // Count retained
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason) {
  Serial.println("MQTT disconnected.");
  if (staReady) { delay(2000); mqttConnect(); }
}
void publishCountIfNeeded() {
  if (mqttEnabled && mqtt.connected()) {
    String p = String(touchCount);
    mqtt.publish(MQTT_TOPIC_COUNT, 2, true, p.c_str());
    Serial.println("Published count @ QoS2 retained");
  }
}

// ================== SETUP / LOOP ==================
void setup() {
  Serial.begin(115200);
  delay(300);

  // Build company SSID with MAC suffix
  String svcSsid = String("ESP32_TouchCounter-SVC-") + macSuffix4();
  svcSsid.toCharArray(ap_ssid_company, sizeof(ap_ssid_company));

  // One-shot force flag (set by touch double-tap before reboot)
  prefs.begin("svc", true);
  forceCompany = prefs.getBool("force", false);
  prefs.end();

  // DRD detection (reset double-press)
  bool drdCompany = drd.detectDoubleReset();

  // Final mode decision: DRD OR force flag
  companyMode = drdCompany || forceCompany;

  // If force flag used once, clear it
  if (forceCompany) {
    prefs.begin("svc", false);
    prefs.putBool("force", false);
    prefs.end();
  }

  Serial.printf("Boot mode: %s (DRD=%d, force=%d)\n", companyMode ? "COMPANY" : "customer", (int)drdCompany, (int)forceCompany);

  // Load saved Wi-Fi creds (used only in customer mode)
  loadWifiCreds();

  if (companyMode) {
    // ---------------- COMPANY MODE ----------------
    startAp(ap_ssid_company, AP_PASS_COMPANY);

    // Routes for company UI
    server.on("/", HTTP_GET, handleRootCompany);
    server.on("/svc/ping", HTTP_POST, handleSvcPing);
    server.onNotFound([](){ server.send(404, "text/plain", "Not found (company)"); });
    server.begin();
    Serial.println("Company HTTP server started. Open http://192.168.4.1/");
  } else {
    // ---------------- CUSTOMER MODE ----------------
    startApStaCustomer();

    server.on("/", HTTP_GET, handleRootCustomer);
    server.on("/api/state", HTTP_GET, handleApiState);
    server.on("/api/reset", HTTP_POST, handleApiReset);
    server.on("/api/mqtt", HTTP_POST, handleApiMqtt);
    server.on("/api/threshold", HTTP_POST, handleApiThreshold);
    server.on("/api/wifi", HTTP_POST, handleApiWifi);
    server.on("/api/disconnect", HTTP_POST, handleApiDisconnect);
    server.onNotFound([](){ server.send(404, "text/plain", "Not found"); });
    server.begin();
    Serial.println("Customer HTTP server started. Open http://192.168.4.1/");

    // MQTT only in customer mode
    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    mqttConnect();

    Serial.println("Touch counter initialized (customer mode).");
  }
}

void loop() {
  drd.loop();               // keep DRD service alive
  server.handleClient();    // serve dashboard(s)

  // Double-tap on GPIO14 (T6) to enter Company Mode (no reset button needed)
  if (!companyMode && svcDoubleTapDetected()) {
    Preferences p;
    p.begin("svc", false);
    p.putBool("force", true);     // one-shot flag used at next boot
    p.end();
    Serial.println("Service double-tap detected -> rebooting to Company Mode");
    delay(200);
    ESP.restart();
  }

  // Touch counter only in customer mode
  if (!companyMode) {
    int touchVal = touchRead(TOUCH_PIN);
    if (touchVal < TOUCH_THRESHOLD && !wasTouched) {
      wasTouched = true;
      touchCount++;
      publishCountIfNeeded();
    } else if (touchVal >= TOUCH_THRESHOLD) {
      wasTouched = false;
    }
  }

  delay(200); // debounce / pacing
}