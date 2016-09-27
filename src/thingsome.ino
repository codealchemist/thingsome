#include <Arduino.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <info.h>
#include <index.h>
#pragma GCC diagnostic ignored "-Wwrite-strings"

// register events
WiFiEventHandler onSoftAPModeStationConnected;
WiFiEventHandler onSoftAPModeStationDisconnected;

#define LED_PIN 5
String station_ssid = "";
String station_psk = "";

// set server on port 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // start wifi
  Serial.println(ASCII);
  setupWifi();

  // start web server
  server.begin();
  server.setNoDelay(true);
  Serial.println(F("Web server started!"));
  Serial.println(F("Waiting for clients..."));
}

void setupWifi() {
  // try to start in station mode by default
  // if not possible start access point
  if (!startStation()) {
      startAccessPoint();
  }
}

void loop() {
  WiFiClient client = server.available();  // Check if a client has connected
  if (!client) return;

  while (client.connected()) {
    if (!client.available()) continue;

    // read the request one line at a time
    String req = client.readStringUntil('\r');
    if (isFaviconRequest(&req)) return; // ignore favicon request
    // Serial.println(req);

    // setup
    if (isSetupRequest(&req)) {
      Serial.println(F("SETUP"));
      setStationConfig(&req); // get and save config
      client.println(INDEX("Setup completed"));
    }

    // reset
    if (isResetRequest(&req)) {
      Serial.println(F("RESET"));
      resetStationConfig();
      startAccessPoint();
      client.println(INDEX("Device reset completed"));
    }

    // retry
    if (isRetryRequest(&req)) {
      Serial.println(F("RETRY LOGIN"));
      client.println(INDEX("Retrying login to " + station_ssid));
      if (!startStation()) {
        startAccessPoint();
      }
    }

    // start access point
    if (isStartAccessPointRequest(&req)) {
      Serial.println(F("START AP"));
      client.println(INDEX("Starting Access Point"));
      startAccessPoint();
      break; // no need to continue processing request
    }

    if (isDisconnectStationRequest(&req)) {
      Serial.println(F("DISCONNECT STATION"));
      WiFi.disconnect(true);
      client.println(INDEX("Disconnected from " + station_ssid));
    }

    if (isInfoRequest(&req)) {
      Serial.println(F("RETURN DEVICE INFO"));
      client.println(INFO());
      break; // no need to continue processing request
    }

    // wait for end of client's request, that is marked with an empty line
    if (req.length() == 1 && req[0] == '\n') {
      client.println(INDEX(""));
      break; // no need to continue processing request, already ended
    }
  }

  printLine();
  delay(1);
  client.stop();
  Serial.println(F("Client disonnected"));

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

void printLine() {
  Serial.println(F("--------------------------------------------------"));
}

bool isSetupRequest(String *req) {
  // sample setup request: GET /setup?ssid=code&pass=test1234 HTTP/1.1
  if (
    req->indexOf("GET /setup?") != -1 &&
    req->indexOf("ssid=") != -1 &&
    req->indexOf("pass=") != -1
  ) {
    return true;
  }

  return false;
}

bool isResetRequest(String *req) {
  // sample setup request: GET /reset-config HTTP/1.1
  if (req->indexOf("GET /reset-config") != -1) return true;
  return false;
}

bool isRetryRequest(String *req) {
  // sample setup request: GET /retry-wifi-login HTTP/1.1
  if (req->indexOf("GET /retry-wifi-login") != -1) return true;
  return false;
}

bool isStartAccessPointRequest(String *req) {
  // sample setup request: GET /start-access-point HTTP/1.1
  if (req->indexOf("GET /start-access-point") != -1) return true;
  return false;
}

bool isDisconnectStationRequest(String *req) {
  // sample setup request: GET /disconnect-station HTTP/1.1
  if (req->indexOf("GET /disconnect-station") != -1) return true;
  return false;
}

bool isInfoRequest(String *req) {
  // sample setup request: GET /device-info HTTP/1.1
  if (req->indexOf("GET /device-info") != -1) return true;
  return false;
}

bool isFaviconRequest(String *req) {
  // sample setup request: GET /favicon.ico HTTP/1.1
  if (req->indexOf("GET /favicon.ico") != -1) return true;
  return false;
}

// TODO: write a generic function to get query string params
String getSsid(String *req) {
  String ssidParam = "ssid=";
  uint start = req->indexOf(ssidParam) + ssidParam.length();
  uint end = req->indexOf("&pass=");
  return req->substring(start, end);
}

String getPass(String *req) {
  String passParam = "pass=";
  uint start = req->indexOf(passParam) + passParam.length();
  uint end = req->indexOf(" HTTP");
  return req->substring(start, end);
}

void setStationConfig(String *req) {
  Serial.println(F("Set station config:"));
  String ssid = getSsid(req);
  String pass = getPass(req);
  Serial.println("-- SSID: " + ssid);
  Serial.println("-- PASS: " + pass);
  saveConfig(&ssid, &pass);
}

void resetStationConfig() {
  Serial.println(F("Removing WIFI config..."));
  SPIFFS.remove("/config.txt");
}

void startAccessPoint() {
  String ssid = "IOT-DEVICE-";
  char pass[] = "----iotfun"; // min 8 chars or the AP is initialized with default values!
  Serial.println(F("Starting AP..."));
  Serial.println(F("Default AP IP: 192.168.4.1"));
  WiFi.mode(WIFI_AP);

  // listen to wifi events
  onSoftAPModeStationConnected = WiFi.onSoftAPModeStationConnected([](const WiFiEventSoftAPModeStationConnected& event) {
    Serial.println(F("Client connected to AP"));
  });
  onSoftAPModeStationDisconnected = WiFi.onSoftAPModeStationDisconnected([](const WiFiEventSoftAPModeStationDisconnected& event) {
    Serial.println(F("Client disconnected from AP"));
  });

  // Append the last two bytes of the MAC (HEX'd) to string to make unique
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);

  macID.toUpperCase();
  String uniqueSsid = ssid + macID;

  WiFi.hostname(uniqueSsid);
  WiFi.softAP(uniqueSsid.c_str(), pass);
}

bool startStation() {
  // Initialize file system.
  if (!SPIFFS.begin()) {
    Serial.println(F("Failed to mount file system"));
    return false;
  }

  // Load wifi connection information.
  if (!loadConfig(&station_ssid, &station_psk)) {
    station_ssid = "";
    station_psk = "";

    Serial.println("WIFI login data not available.");
    return false;
  }

  // config loaded, start station
  Serial.println("Connecting to " + station_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(station_ssid.c_str(), station_psk.c_str());

  // Give ESP 10 seconds to connect to station
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    Serial.write('.');
    delay(500);
  }
  Serial.println("\n");

  // unable to connect
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Unable to connect to " + station_ssid);
    return false;
  }

  // connected OK!
  Serial.println("Connected to " + station_ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

/**
 * @brief Read WiFi connection information from file system.
 * @param ssid String pointer for storing SSID.
 * @param pass String pointer for storing PSK.
 * @return True or False.
 *
 * The config file have to containt the WiFi SSID in the first line
 * and the WiFi PSK in the second line.
 * Line seperator can be \r\n (CR LF) \r or \n.
 */
bool loadConfig(String *ssid, String *pass) {
  // open file for reading.
  File configFile = SPIFFS.open("/config.txt", "r");
  if (!configFile) {
    Serial.println("Failed to open config.");
    return false;
  }

  // Read content from config file.
  String content = configFile.readString();
  configFile.close();
  content.trim();

  // Check if ther is a second line available.
  int8_t pos = content.indexOf("\r\n");
  uint8_t le = 2;
  // check for linux and mac line ending.
  if (pos == -1) {
    le = 1;
    pos = content.indexOf("\n");
    if (pos == -1) {
      pos = content.indexOf("\r");
    }
  }

  // If there is no second line: Some information is missing.
  if (pos == -1) {
    Serial.println("Invalid config.");
    Serial.println(content);

    return false;
  }

  // Store SSID and PSK into string vars.
  *ssid = content.substring(0, pos);
  *pass = content.substring(pos + le);

  ssid->trim();
  pass->trim();

  // Serial.println("----- file content -----");
  // Serial.println(content);
  // Serial.println("----- file content -----");
  // Serial.println("ssid: " + *ssid);
  // Serial.println("psk:  " + *pass);

  return true;
} // loadConfig


/**
 * @brief Save WiFi SSID and PSK to configuration file.
 * @param ssid SSID as string pointer.
 * @param pass PSK as string pointer,
 * @return True or False.
 */
bool saveConfig(String *ssid, String *pass) {
  Serial.println(F("Saving config..."));
  // Open config file for writing.
  File configFile = SPIFFS.open("/config.txt", "w");
  if (!configFile) {
    Serial.println("Failed to open config.txt for writing");
    return false;
  }

  // Save SSID and PSK.
  configFile.println(*ssid);
  configFile.println(*pass);

  configFile.close();
  return true;
} // saveConfig
