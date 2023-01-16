#include <Arduino.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <webpageCode.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <ESP8266mDNS.h>
#include <W5500lwIP.h>
// ============================================= DEFINISI PIN MIKRO ============================================= //

// ========================================== AKHIR DEFINISI PIN MIKERO ========================================== //

// ============================================= DEFINISI VARIABEL ============================================= //
#ifndef STASSID
#define STASSID "LAUNDRY KURNIA"
#define STAPSK "HARTOYO130606"
#endif

#define EEPROM_SIZE 1024
// #define STATIC

String pubTopic = "alert/device/";
String subTopic = "alert/server/";

#define MYPORT_TX D3
#define MYPORT_RX D4
SoftwareSerial Serial2;

const char *ssid = STASSID;
const char *password = STAPSK;

byte
    val_led = 0,
    tick = 0,             // TICK SCHEDULER
    detik = 0,            // DETIK
    menit = 0,            // MENIT
    jam = 0,              // JAM
    TIMEOUT_MQTT_ETH = 0, // TIMEOUT MQTT ETH
    TIMEOUT_SERIAL0 = 10, // TIMEOUT SERIAL0
    TIMEOUT_SERIAL1 = 10, // TIMEOUT SERIAL1
    TIMEOUT_ETHERNET = 0, // TIMEOUT KONEKSI ETHERNET
    SERIAL0_FAIL = 0,     // FLAG SERIAL0 FAIL
    SERIAL1_FAIL = 0;     // FLAG SERIAL1 FAIL

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

#define CSPIN 16

ESP8266WebServer wifiServer(80);
EthernetServer ethServer(80);
Wiznet5500lwIP eth(CSPIN);

const char *mqtt_server = "192.168.100.30";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
String msg;
String payload_svr;
String serial_data;
int value = 0;

bool
    ETH_MODE = true,
    WIFI_ON_SETUP = false,
    DEBUG_MODE = true;

int
    TIMEOUT_PAYLOAD_BUFFER = 0;

int
    IP_ARR[5] = {192, 168, 100, 103},
    ADDR_IP_ARR[5] = {0, 1, 2, 3},
    NETMASK_ARR[5] = {255, 255, 255, 0},
    ADDR_NETMASK_ARR[5] = {4, 5, 6, 7},
    GATEWAY_ARR[5] = {192, 168, 100, 1},
    ADDR_GATEWAY_ARR[5] = {8, 9, 10, 11},
    DNS_ARR[5] = {192, 168, 100, 1},
    ADDR_DNS_ARR[5] = {12, 13, 14, 15};

void callback(char *topic, byte *payload, unsigned int length)
{
  if (DEBUG_MODE)
  {
    Serial.print("DEBUG_SERVER -> [");
    Serial.print(subTopic + "], ");
  }

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    payload_svr += (char)payload[i];
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  if ((!WIFI_ON_SETUP) && (!ETH_MODE))
  {
    IPAddress IP(IP_ARR[0], IP_ARR[1], IP_ARR[2], IP_ARR[3]);
    IPAddress NETMASK(NETMASK_ARR[0], NETMASK_ARR[1], NETMASK_ARR[2], NETMASK_ARR[3]);
    IPAddress GATEWAY(GATEWAY_ARR[0], GATEWAY_ARR[1], GATEWAY_ARR[2], GATEWAY_ARR[3]);
    IPAddress DNS(DNS_ARR[0], DNS_ARR[1], DNS_ARR[2], DNS_ARR[3]);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.config(IP, GATEWAY, NETMASK, DNS);
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.println("Reconnecting To Network ...");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "PIT_ALERT_SYSTEM-" + String(IP_ARR[3]);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      msg = "PIT_ALERT_SYSTEM-" + String(IP_ARR[3]);
      client.publish(pubTopic.c_str(), msg.c_str());
      // ... and resubscribe
      subTopic += String(IP_ARR[3]);
      client.subscribe(subTopic.c_str());
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      if (ETH_MODE)
      {
        TIMEOUT_MQTT_ETH++;
        if (TIMEOUT_MQTT_ETH > 4)
        {
          ETH_MODE = false;
          IPAddress IP(IP_ARR[0], IP_ARR[1], IP_ARR[2], IP_ARR[3]);
          IPAddress NETMASK(NETMASK_ARR[0], NETMASK_ARR[1], NETMASK_ARR[2], NETMASK_ARR[3]);
          IPAddress GATEWAY(GATEWAY_ARR[0], GATEWAY_ARR[1], GATEWAY_ARR[2], GATEWAY_ARR[3]);
          IPAddress DNS(DNS_ARR[0], DNS_ARR[1], DNS_ARR[2], DNS_ARR[3]);
          WiFi.mode(WIFI_STA);
          WiFi.begin(ssid, password);
          WiFi.config(IP, GATEWAY, NETMASK, DNS);
          Serial.println("");

          while (WiFi.status() != WL_CONNECTED)
          {
            delay(500);
            Serial.println("Connecting To Network ...");
          }
          Serial.println("");
          Serial.print("Connected to ");
          Serial.println(ssid);
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          client.setServer(mqtt_server, 1883);
          client.setCallback(callback);
        }
      }
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

const int led = 13;

// void webpage()
// {
//   server.send(200, "text/html", webpageCode);
// }

// Check if header is present and correct
bool is_authenticated()
{
  Serial.println("Enter is_authenticated");
  if (wifiServer.hasHeader("Cookie"))
  {
    Serial.print("Found cookie: ");
    String cookie = wifiServer.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1)
    {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

// login page, also called for disconnect
void handleLogin()
{
  String msg;
  if (wifiServer.hasHeader("Cookie"))
  {
    Serial.print("Found cookie: ");
    String cookie = wifiServer.header("Cookie");
    Serial.println(cookie);
  }
  if (wifiServer.hasArg("DISCONNECT"))
  {
    Serial.println("Disconnection");
    wifiServer.sendHeader("Location", "/login");
    wifiServer.sendHeader("Cache-Control", "no-cache");
    wifiServer.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    wifiServer.send(301);
    return;
  }
  if (wifiServer.hasArg("IP"))
  {
    // ============================== DEBUG =============================== //
    Serial.println(wifiServer.arg("IP"));
    String IP_WEBSERVER = wifiServer.arg("IP");
    byte
        x1 = IP_WEBSERVER.indexOf("."),
        x2 = IP_WEBSERVER.indexOf(".", x1 + 1),
        x3 = IP_WEBSERVER.indexOf(".", x2 + 1);

    IP_ARR[0] = (IP_WEBSERVER.substring(0, x1)).toInt();
    IP_ARR[1] = (IP_WEBSERVER.substring(x1 + 1, x2)).toInt();
    IP_ARR[2] = (IP_WEBSERVER.substring(x2 + 1, x3)).toInt();
    IP_ARR[3] = (IP_WEBSERVER.substring(x3 + 1, x3 + 4)).toInt();

    for (byte i = 0; i < 4; i++)
    {
      switch (i)
      {
      case 0:
        if ((IP_ARR[0] == 192) || (IP_ARR[0] == 172))
        {
          EEPROM.write(ADDR_IP_ARR[0], IP_ARR[0]);
          EEPROM.commit();
        }
        break;
      case 1:
        if ((IP_ARR[1] == 168) || (IP_ARR[1] == 16))
        {
          EEPROM.write(ADDR_IP_ARR[1], IP_ARR[1]);
          EEPROM.commit();
        }
        break;
      case 2:
        if ((IP_ARR[2] < 255))
        {
          EEPROM.write(ADDR_IP_ARR[2], IP_ARR[2]);
          EEPROM.commit();
        }
        break;
      case 3:
        if ((IP_ARR[3] < 1000))
        {
          EEPROM.write(ADDR_IP_ARR[3], IP_ARR[3]);
          EEPROM.commit();
        }
        break;
      default:
        break;
      }
    }

    Serial.println(wifiServer.arg("NETMASK"));
    String NETMASK_WEBSERVER = wifiServer.arg("NETMASK");
    byte
        x1_b = NETMASK_WEBSERVER.indexOf("."),
        x2_b = NETMASK_WEBSERVER.indexOf(".", x1_b + 1),
        x3_b = NETMASK_WEBSERVER.indexOf(".", x2_b + 1);

    NETMASK_ARR[0] = (NETMASK_WEBSERVER.substring(0, x1_b)).toInt();
    NETMASK_ARR[1] = (NETMASK_WEBSERVER.substring(x1_b + 1, x2_b)).toInt();
    NETMASK_ARR[2] = (NETMASK_WEBSERVER.substring(x2_b + 1, x3_b)).toInt();
    NETMASK_ARR[3] = (NETMASK_WEBSERVER.substring(x3_b + 1, x3_b + 4)).toInt();

    for (byte i = 0; i < 4; i++)
    {
      switch (i)
      {
      case 0:
        if ((NETMASK_ARR[0] == 192) || (NETMASK_ARR[0] == 172))
        {
          EEPROM.write(ADDR_NETMASK_ARR[0], NETMASK_ARR[0]);
          EEPROM.commit();
        }
        break;
      case 1:
        if ((NETMASK_ARR[1] == 168) || (NETMASK_ARR[1] == 16))
        {
          EEPROM.write(ADDR_NETMASK_ARR[1], NETMASK_ARR[1]);
          EEPROM.commit();
        }
        break;
      case 2:
        if ((NETMASK_ARR[2] < 255))
        {
          EEPROM.write(ADDR_NETMASK_ARR[2], NETMASK_ARR[2]);
          EEPROM.commit();
        }
        break;
      case 3:
        if ((NETMASK_ARR[3] < 1000))
        {
          EEPROM.write(ADDR_NETMASK_ARR[3], NETMASK_ARR[3]);
          EEPROM.commit();
        }
        break;
      default:
        break;
      }
      // ======================= DEBUG ========================== //
      // Serial.println(NETMASK_ARR[i]);
    }

    Serial.println(wifiServer.arg("GATEWAY"));
    String GATEWAY_WEBSERVER = wifiServer.arg("GATEWAY");
    byte
        x1_c = GATEWAY_WEBSERVER.indexOf("."),
        x2_c = GATEWAY_WEBSERVER.indexOf(".", x1_c + 1),
        x3_c = GATEWAY_WEBSERVER.indexOf(".", x2_c + 1);

    GATEWAY_ARR[0] = (GATEWAY_WEBSERVER.substring(0, x1_c)).toInt();
    GATEWAY_ARR[1] = (GATEWAY_WEBSERVER.substring(x1_c + 1, x2_c)).toInt();
    GATEWAY_ARR[2] = (GATEWAY_WEBSERVER.substring(x2_c + 1, x3_c)).toInt();
    GATEWAY_ARR[3] = (GATEWAY_WEBSERVER.substring(x3_c + 1, x3_c + 4)).toInt();

    for (byte i = 0; i < 4; i++)
    {
      switch (i)
      {
      case 0:
        if ((GATEWAY_ARR[0] == 192) || (GATEWAY_ARR[0] == 172))
        {
          EEPROM.write(ADDR_GATEWAY_ARR[0], GATEWAY_ARR[0]);
          EEPROM.commit();
        }
        break;
      case 1:
        if ((GATEWAY_ARR[1] == 168) || (GATEWAY_ARR[1] == 16))
        {
          EEPROM.write(ADDR_GATEWAY_ARR[1], GATEWAY_ARR[1]);
          EEPROM.commit();
        }
        break;
      case 2:
        if ((GATEWAY_ARR[2] < 255))
        {
          EEPROM.write(ADDR_GATEWAY_ARR[2], GATEWAY_ARR[2]);
          EEPROM.commit();
        }
        break;
      case 3:
        if ((GATEWAY_ARR[3] < 1000))
        {
          EEPROM.write(ADDR_GATEWAY_ARR[3], GATEWAY_ARR[3]);
          EEPROM.commit();
        }
        break;
      default:
        break;
      }
      // ======================= DEBUG ========================== //
      // Serial.println(GATEWAY_ARR[i]);
    }

    Serial.println(wifiServer.arg("DNS"));
    String DNS_WEBSERVER = wifiServer.arg("DNS");
    byte
        x1_d = DNS_WEBSERVER.indexOf("."),
        x2_d = DNS_WEBSERVER.indexOf(".", x1_d + 1),
        x3_d = DNS_WEBSERVER.indexOf(".", x2_d + 1);

    DNS_ARR[0] = (DNS_WEBSERVER.substring(0, x1_d)).toInt();
    DNS_ARR[1] = (DNS_WEBSERVER.substring(x1_d + 1, x2_d)).toInt();
    DNS_ARR[2] = (DNS_WEBSERVER.substring(x2_d + 1, x3_d)).toInt();
    DNS_ARR[3] = (DNS_WEBSERVER.substring(x3_d + 1, x3 + 4)).toInt();

    for (byte i = 0; i < 4; i++)
    {
      switch (i)
      {
      case 0:
        if ((DNS_ARR[0] == 192) || (DNS_ARR[0] == 172))
        {
          EEPROM.write(ADDR_DNS_ARR[0], DNS_ARR[0]);
          EEPROM.commit();
        }
        break;
      case 1:
        if ((DNS_ARR[1] == 168) || (DNS_ARR[1] == 16))
        {
          EEPROM.write(ADDR_DNS_ARR[1], DNS_ARR[1]);
          EEPROM.commit();
        }
        break;
      case 2:
        if ((DNS_ARR[2] < 255))
        {
          EEPROM.write(ADDR_DNS_ARR[2], DNS_ARR[2]);
          EEPROM.commit();
        }
        break;
      case 3:
        if ((DNS_ARR[3] < 1000))
        {
          EEPROM.write(ADDR_DNS_ARR[3], DNS_ARR[3]);
          EEPROM.commit();
        }
        break;
      default:
        break;
      }
      // ======================= DEBUG ========================== //
      // Serial.println(DNS_ARR[i]);
    }
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "IP:<input type='text' name='IP' placeholder='IP'><br>";
  content += "NETMASK:<input type='text' name='NETMASK' placeholder='NETMASK'><br>";
  content += "GATEWAY:<input type='text' name='GATEWAY' placeholder='GATEWAY'><br>";
  content += "DNS:<input type='text' name='DNS' placeholder='DNS'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  // content += "You also can go <a href='/inline'>here</a></body></html>";
  wifiServer.send(200, "text/html", content);
}

// root page can be accessed only if authentication is ok
void handleRoot()
{
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated())
  {
    wifiServer.sendHeader("Location", "/login");
    wifiServer.sendHeader("Cache-Control", "no-cache");
    wifiServer.send(301);
    return;
  }
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  if (wifiServer.hasHeader("User-Agent"))
  {
    content += "the user agent used is : " + wifiServer.header("User-Agent") + "<br><br>";
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  wifiServer.send(200, "text/html", content);
}

// no need authentication
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += wifiServer.uri();
  message += "\nMethod: ";
  message += (wifiServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += wifiServer.args();
  message += "\n";
  for (uint8_t i = 0; i < wifiServer.args(); i++)
  {
    message += " " + wifiServer.argName(i) + ": " + wifiServer.arg(i) + "\n";
  }
  wifiServer.send(404, "text/plain", message);
}

// ========================================== AKHIR DEFINISI VARIABEL ========================================== //

void serial()
{
  wifiServer.handleClient();
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  if (Serial.available() > 0)
  {
    serial_data = Serial.readStringUntil('}');
    serial_data += "}";
  }
}

void proses()
{
  tick++;
  // ============================================= PENGULANGAN 10ms ============================================= //
  if ((payload_svr.indexOf("{") > -1) && (payload_svr.indexOf("}") > -1))
  {
    const size_t capacity_mqtt_time = JSON_OBJECT_SIZE(9) + 40;
    DynamicJsonDocument doc_mqtt(capacity_mqtt_time);
    deserializeJson(doc_mqtt, payload_svr.c_str());
    String FUNC = doc_mqtt["FUNC"];
    String DATA = doc_mqtt["DATA"];
    // if (FUNC == "ALARM")
    // {
    // if (DATA == "MD1")
    // {
    Serial.println(payload_svr);
    Serial2.print(payload_svr);
    // }
    // }

    // ======================= DEBUG ========================== //
    if (DEBUG_MODE)
    {
      Serial.print("DEBUG_DEVICE -> [ALERT_CODE], ");
      Serial.println(DATA);
    }

    payload_svr = "";
  }

  if ((serial_data.indexOf("{") > -1) && (serial_data.indexOf("}") > -1))
  {
    const size_t capacity_mqtt_time = JSON_OBJECT_SIZE(9) + 40;
    DynamicJsonDocument doc_mqtt(capacity_mqtt_time);
    deserializeJson(doc_mqtt, serial_data.c_str());
    String SET = doc_mqtt["SET"];
    String FUNC = doc_mqtt["FUNC"];
    String DATA = doc_mqtt["DATA"];
    // ======================= DEBUG ========================== //
    if (SET == "IP")
    {
      byte
          x1 = DATA.indexOf("."),
          x2 = DATA.indexOf(".", x1 + 1),
          x3 = DATA.indexOf(".", x2 + 1);

      IP_ARR[0] = (DATA.substring(0, x1)).toInt();
      IP_ARR[1] = (DATA.substring(x1 + 1, x2)).toInt();
      IP_ARR[2] = (DATA.substring(x2 + 1, x3)).toInt();
      IP_ARR[3] = (DATA.substring(x3 + 1, x3 + 4)).toInt();

      for (byte i = 0; i < 4; i++)
      {
        switch (i)
        {
        case 0:
          if ((IP_ARR[0] == 192) || (IP_ARR[0] == 172))
          {
            EEPROM.write(ADDR_IP_ARR[0], IP_ARR[0]);
            EEPROM.commit();
          }
          break;
        case 1:
          if ((IP_ARR[1] == 168) || (IP_ARR[1] == 16))
          {
            EEPROM.write(ADDR_IP_ARR[1], IP_ARR[1]);
            EEPROM.commit();
          }
          break;
        case 2:
          if ((IP_ARR[2] < 255))
          {
            EEPROM.write(ADDR_IP_ARR[2], IP_ARR[2]);
            EEPROM.commit();
          }
          break;
        case 3:
          if ((IP_ARR[3] < 1000))
          {
            EEPROM.write(ADDR_IP_ARR[3], IP_ARR[3]);
            EEPROM.commit();
          }
          break;
        default:
          break;
        }
        // ======================= DEBUG ========================== //
        // Serial.println(IP_ARR[i]);
      }

      if (DEBUG_MODE)
      {
        Serial.print("DEBUG_DEVICE -> [SET_IP], ");
        Serial.print(String(IP_ARR[0]) + ".");
        Serial.print(String(IP_ARR[1]) + ".");
        Serial.print(String(IP_ARR[2]) + ".");
        Serial.println(String(IP_ARR[3]));
      }
      Serial.println(serial_data);
    }
    else if (SET == "NETMASK")
    {
      byte
          x1 = DATA.indexOf("."),
          x2 = DATA.indexOf(".", x1 + 1),
          x3 = DATA.indexOf(".", x2 + 1);

      NETMASK_ARR[0] = (DATA.substring(0, x1)).toInt();
      NETMASK_ARR[1] = (DATA.substring(x1 + 1, x2)).toInt();
      NETMASK_ARR[2] = (DATA.substring(x2 + 1, x3)).toInt();
      NETMASK_ARR[3] = (DATA.substring(x3 + 1, x3 + 4)).toInt();

      for (byte i = 0; i < 4; i++)
      {
        switch (i)
        {
        case 0:
          if ((NETMASK_ARR[0] == 192) || (NETMASK_ARR[0] == 172))
          {
            EEPROM.write(ADDR_NETMASK_ARR[0], NETMASK_ARR[0]);
            EEPROM.commit();
          }
          break;
        case 1:
          if ((NETMASK_ARR[1] == 168) || (NETMASK_ARR[1] == 16))
          {
            EEPROM.write(ADDR_NETMASK_ARR[1], NETMASK_ARR[1]);
            EEPROM.commit();
          }
          break;
        case 2:
          if ((NETMASK_ARR[2] < 255))
          {
            EEPROM.write(ADDR_NETMASK_ARR[2], NETMASK_ARR[2]);
            EEPROM.commit();
          }
          break;
        case 3:
          if ((NETMASK_ARR[3] < 1000))
          {
            EEPROM.write(ADDR_NETMASK_ARR[3], NETMASK_ARR[3]);
            EEPROM.commit();
          }
          break;
        default:
          break;
        }
        // ======================= DEBUG ========================== //
        // Serial.println(NETMASK_ARR[i]);
      }

      if (DEBUG_MODE)
      {
        Serial.print("DEBUG_DEVICE -> [SET_NETMASK], ");
        Serial.print(String(NETMASK_ARR[0]) + ".");
        Serial.print(String(NETMASK_ARR[1]) + ".");
        Serial.print(String(NETMASK_ARR[2]) + ".");
        Serial.println(String(NETMASK_ARR[3]));
      }
      Serial.println(serial_data);
    }
    else if (SET == "GATEWAY")
    {
      byte
          x1 = DATA.indexOf("."),
          x2 = DATA.indexOf(".", x1 + 1),
          x3 = DATA.indexOf(".", x2 + 1);

      GATEWAY_ARR[0] = (DATA.substring(0, x1)).toInt();
      GATEWAY_ARR[1] = (DATA.substring(x1 + 1, x2)).toInt();
      GATEWAY_ARR[2] = (DATA.substring(x2 + 1, x3)).toInt();
      GATEWAY_ARR[3] = (DATA.substring(x3 + 1, x3 + 4)).toInt();

      for (byte i = 0; i < 4; i++)
      {
        switch (i)
        {
        case 0:
          if ((GATEWAY_ARR[0] == 192) || (GATEWAY_ARR[0] == 172))
          {
            EEPROM.write(ADDR_GATEWAY_ARR[0], GATEWAY_ARR[0]);
            EEPROM.commit();
          }
          break;
        case 1:
          if ((GATEWAY_ARR[1] == 168) || (GATEWAY_ARR[1] == 16))
          {
            EEPROM.write(ADDR_GATEWAY_ARR[1], GATEWAY_ARR[1]);
            EEPROM.commit();
          }
          break;
        case 2:
          if ((GATEWAY_ARR[2] < 255))
          {
            EEPROM.write(ADDR_GATEWAY_ARR[2], GATEWAY_ARR[2]);
            EEPROM.commit();
          }
          break;
        case 3:
          if ((GATEWAY_ARR[3] < 1000))
          {
            EEPROM.write(ADDR_GATEWAY_ARR[3], GATEWAY_ARR[3]);
            EEPROM.commit();
          }
          break;
        default:
          break;
        }
        // ======================= DEBUG ========================== //
        // Serial.println(GATEWAY_ARR[i]);
      }

      if (DEBUG_MODE)
      {
        Serial.print("DEBUG_DEVICE -> [SET_GATEWAY], ");
        Serial.print(String(GATEWAY_ARR[0]) + ".");
        Serial.print(String(GATEWAY_ARR[1]) + ".");
        Serial.print(String(GATEWAY_ARR[2]) + ".");
        Serial.println(String(GATEWAY_ARR[3]));
      }
      Serial.println(serial_data);
    }
    else if (SET == "DNS")
    {
      byte
          x1 = DATA.indexOf("."),
          x2 = DATA.indexOf(".", x1 + 1),
          x3 = DATA.indexOf(".", x2 + 1);

      DNS_ARR[0] = (DATA.substring(0, x1)).toInt();
      DNS_ARR[1] = (DATA.substring(x1 + 1, x2)).toInt();
      DNS_ARR[2] = (DATA.substring(x2 + 1, x3)).toInt();
      DNS_ARR[3] = (DATA.substring(x3 + 1, x3 + 4)).toInt();

      for (byte i = 0; i < 4; i++)
      {
        switch (i)
        {
        case 0:
          if ((DNS_ARR[0] == 192) || (DNS_ARR[0] == 172))
          {
            EEPROM.write(ADDR_DNS_ARR[0], DNS_ARR[0]);
            EEPROM.commit();
          }
          break;
        case 1:
          if ((DNS_ARR[1] == 168) || (DNS_ARR[1] == 16))
          {
            EEPROM.write(ADDR_DNS_ARR[1], DNS_ARR[1]);
            EEPROM.commit();
          }
          break;
        case 2:
          if ((DNS_ARR[2] < 255))
          {
            EEPROM.write(ADDR_DNS_ARR[2], DNS_ARR[2]);
            EEPROM.commit();
          }
          break;
        case 3:
          if ((DNS_ARR[3] < 1000))
          {
            EEPROM.write(ADDR_DNS_ARR[3], DNS_ARR[3]);
            EEPROM.commit();
          }
          break;
        default:
          break;
        }
        // ======================= DEBUG ========================== //
        // Serial.println(DNS_ARR[i]);
      }

      if (DEBUG_MODE)
      {
        Serial.print("DEBUG_DEVICE -> [SET_DNS], ");
        Serial.print(String(DNS_ARR[0]) + ".");
        Serial.print(String(DNS_ARR[1]) + ".");
        Serial.print(String(DNS_ARR[2]) + ".");
        Serial.println(String(DNS_ARR[3]));
      }
      Serial.println(serial_data);
    }
    serial_data = "";
  }

  // ========================================== AKHIR PENGULANGAN 10ms ========================================== //
  if (tick % 10 == 0)
  {
    // ============================================= PENGULANGAN 100ms ============================================= //
    if (val_led == 1)
    {
      val_led = 0;
    }
    else
    {
      val_led = 1;
    }
    digitalWrite(LED_BUILTIN, val_led);
    // ========================================== AKHIR PENGULANGAN 100ms ========================================== //
  }
  switch (tick)
  {
  case 100:
    tick = 0;
    // ============================================= PENGULANGAN 1 Detik ============================================= //
    // MENGIRIM STATUS AKTIF KE SERVER
    msg = "{\"IP\":\"" + String(IP_ARR[0]) + "." + String(IP_ARR[1]) + "." + String(IP_ARR[2]) + "." + String(IP_ARR[3]);
    msg += "\",\"status\":\"online\"}";
    if (DEBUG_MODE)
    {
      Serial.print("DEBUG_DEVICE -> [");
      Serial.print(pubTopic + "], ");
      Serial.println(msg);
    }
    client.publish(pubTopic.c_str(), msg.c_str());

    // TIMEOUT HANDLING
    if (payload_svr != "")
    {
      TIMEOUT_PAYLOAD_BUFFER++;
      if (DEBUG_MODE)
      {
        Serial.print("DEBUG_DEVICE -> [TIMEOUT_PAYLOAD_BUFFER], ");
        Serial.println(TIMEOUT_PAYLOAD_BUFFER);
      }
      if (TIMEOUT_PAYLOAD_BUFFER >= 10)
      {
        payload_svr = "";
        TIMEOUT_PAYLOAD_BUFFER = 0;
        if (DEBUG_MODE)
        {
          Serial.print("DEBUG_DEVICE -> [TIMEOUT_PAYLOAD_STATUS], ");
          Serial.println("TIMEOUT!!");
        }
      }
    }
    else
    {
      TIMEOUT_PAYLOAD_BUFFER = 0;
    }

    // ========================================== AKHIR PENGULANGAN 1 Detik ========================================== //
    detik++;
    if (detik >= 60)
    {
      detik = 0;
      // ============================================= PENGULANGAN 1 Menit ============================================= //

      // ========================================== AKHIR PENGULANGAN 1 Menit ========================================== //
      menit++;
      if (menit >= 60)
      {
        menit = 0;
        // ============================================= PENGULANGAN 1 Jam ============================================= //

        // ========================================== AKHIR PENGULANGAN 1 Jam ========================================== //
        jam++;
        if (jam >= 24)
        {
          jam = 0;
          // ============================================= PENGULANGAN 1 Hari ============================================= //

          // ========================================== AKHIR PENGULANGAN 1 Hari======================================== //
        }
      }
    }
    break;
  }
}

Ticker timer_serial(serial, 1, 0);
Ticker timer_proses(proses, 10, 0);

void setup()
{
  // ============================================= INISIASI SERIAL ============================================= //
  EEPROM.begin(EEPROM_SIZE);
  Serial.begin(9600);
  // Serial2.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  pinMode(LED_BUILTIN, OUTPUT); // INISIASI MODE PADA LED_BUILTIN

  // ============================ WRITE MEMORY MANUAL ============================ //
  // EEPROM.write(ADDR_IP_ARR[2], 100);
  // EEPROM.write(ADDR_IP_ARR[3], 231);
  // EEPROM.write(ADDR_NETMASK_ARR[2], 255);
  // EEPROM.write(ADDR_NETMASK_ARR[3], 0);
  // EEPROM.write(ADDR_GATEWAY_ARR[1], 168);
  // EEPROM.write(ADDR_GATEWAY_ARR[2], 100);
  // EEPROM.write(ADDR_GATEWAY_ARR[3], 1);
  // EEPROM.write(ADDR_DNS_ARR[2], 100);
  // EEPROM.write(ADDR_DNS_ARR[3], 1);
  // EEPROM.commit();
  // ========================== AKHIR WRITE MEMORY MANUAL ======================== //

  // MEMORY READING IP
  for (byte i = 0; i < 4; i++)
  {
    IP_ARR[i] = EEPROM.read(ADDR_IP_ARR[i]);
    switch (i)
    {
    case 0:
      if ((IP_ARR[0] != 192) || (IP_ARR[0] != 172))
      {
        IP_ARR[0] = 192;
        EEPROM.write(ADDR_IP_ARR[0], IP_ARR[0]);
        EEPROM.commit();
      }
      break;
    case 1:
      if ((IP_ARR[1] != 168) || (IP_ARR[1] != 16))
      {
        IP_ARR[1] = 168;
        EEPROM.write(ADDR_IP_ARR[1], IP_ARR[1]);
        EEPROM.commit();
      }
      break;
    case 2:
      if ((IP_ARR[2] > 240))
      {
        IP_ARR[2] = 100;
        EEPROM.write(ADDR_IP_ARR[2], IP_ARR[2]);
        EEPROM.commit();
      }
      break;
    case 3:
      if ((IP_ARR[3] > 1000))
      {
        IP_ARR[3] = 103;
        EEPROM.write(ADDR_IP_ARR[3], IP_ARR[3]);
        EEPROM.commit();
      }
      break;
    default:
      break;
    }
    // ======================= DEBUG ========================== //
    Serial.println(IP_ARR[i]);
  }
  // MEMORY READING NETMASK
  for (byte i = 0; i < 4; i++)
  {
    NETMASK_ARR[i] = EEPROM.read(ADDR_NETMASK_ARR[i]);
    switch (i)
    {
    case 0:
      if ((NETMASK_ARR[0] != 255))
      {
        NETMASK_ARR[0] = 255;
        EEPROM.write(ADDR_NETMASK_ARR[0], NETMASK_ARR[0]);
      }
      break;
    case 1:
      if ((NETMASK_ARR[1] != 255))
      {
        NETMASK_ARR[1] = 255;
        EEPROM.write(ADDR_NETMASK_ARR[1], NETMASK_ARR[1]);
      }
      break;
    case 2:
      if ((NETMASK_ARR[2] != 255) || (NETMASK_ARR[2] != 0))
      {
        NETMASK_ARR[2] = 255;
        EEPROM.write(ADDR_NETMASK_ARR[2], NETMASK_ARR[2]);
        EEPROM.commit();
      }
      break;
    case 3:
      if (NETMASK_ARR[3] == 255)
      {
        NETMASK_ARR[3] = 0;
        EEPROM.write(ADDR_NETMASK_ARR[3], NETMASK_ARR[3]);
        EEPROM.commit();
      }
      break;
    default:
      break;
    }
    // ======================= DEBUG ========================== //
    Serial.println(NETMASK_ARR[i]);
  }
  // MEMORY READING GATEWAY
  for (byte i = 0; i < 4; i++)
  {
    GATEWAY_ARR[i] = EEPROM.read(ADDR_GATEWAY_ARR[i]);
    switch (i)
    {
    case 0:
      if ((GATEWAY_ARR[0] != 192) || (GATEWAY_ARR[0] != 172))
      {
        GATEWAY_ARR[0] = 192;
        EEPROM.write(ADDR_GATEWAY_ARR[0], GATEWAY_ARR[0]);
      }
      break;
    case 1:
      if ((GATEWAY_ARR[1] != 168) || (GATEWAY_ARR[1] != 16))
      {
        GATEWAY_ARR[1] = 168;
        EEPROM.write(ADDR_GATEWAY_ARR[1], GATEWAY_ARR[1]);
      }
      break;
    case 2:
      if (GATEWAY_ARR[2] == 255)
      {
        GATEWAY_ARR[2] = 100;
        EEPROM.write(ADDR_GATEWAY_ARR[2], GATEWAY_ARR[2]);
        EEPROM.commit();
      }
      break;
    case 3:
      if (GATEWAY_ARR[3] == 255)
      {
        GATEWAY_ARR[3] = 1;
        EEPROM.write(ADDR_GATEWAY_ARR[3], GATEWAY_ARR[3]);
        EEPROM.commit();
      }
      break;
    default:
      break;
    }
    // ======================= DEBUG ========================== //
    Serial.println(GATEWAY_ARR[i]);
  }
  // MEMORY READING DNS
  for (byte i = 0; i < 4; i++)
  {
    DNS_ARR[i] = EEPROM.read(ADDR_DNS_ARR[i]);
    switch (i)
    {
    case 0:
      if ((DNS_ARR[0] != 192) || (DNS_ARR[0] != 172))
      {
        DNS_ARR[0] = 192;
        EEPROM.write(ADDR_DNS_ARR[0], DNS_ARR[0]);
      }
      break;
    case 1:
      if ((DNS_ARR[1] != 168) || (DNS_ARR[1] != 16))
      {
        DNS_ARR[1] = 168;
        EEPROM.write(ADDR_DNS_ARR[1], DNS_ARR[1]);
      }
      break;
    case 2:
      if (DNS_ARR[2] == 255)
      {
        DNS_ARR[2] = 100;
        EEPROM.write(ADDR_DNS_ARR[2], DNS_ARR[2]);
        EEPROM.commit();
      }
      break;
    case 3:
      if (DNS_ARR[3] == 255)
      {
        DNS_ARR[3] = 1;
        EEPROM.write(ADDR_DNS_ARR[3], DNS_ARR[3]);
        EEPROM.commit();
      }
      break;
    default:
      break;
    }
    // ======================= DEBUG ========================== //
    Serial.println(DNS_ARR[i]);
  }

  Serial.println("=================== PIT ALERT SYSTEM ===================");
  Serial.println("Hardware Version : 1.0");
  Serial.println("Software Version : 1.0");

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(4000000);

  eth.setDefault(); // use ethernet for default route

  // #ifdef STATIC
  IPAddress IP(IP_ARR[0], IP_ARR[1], IP_ARR[2], IP_ARR[3]);
  IPAddress NETMASK(NETMASK_ARR[0], NETMASK_ARR[1], NETMASK_ARR[2], NETMASK_ARR[3]);
  IPAddress GATEWAY(GATEWAY_ARR[0], GATEWAY_ARR[1], GATEWAY_ARR[2], GATEWAY_ARR[3]);
  IPAddress DNS(DNS_ARR[0], DNS_ARR[1], DNS_ARR[2], DNS_ARR[3]);
  // eth.config(IP, GATEWAY, NETMASK);
  // #endif
  int present = eth.begin(mac);
  // eth.end();
  if (!present)
  {
    Serial.println("no ethernet hardware present");
    while (!present)
    {
      TIMEOUT_ETHERNET++;
      if (TIMEOUT_ETHERNET > 20)
      {
        break;
      }
      delay(1000);
    }
  }
  if (TIMEOUT_ETHERNET < 21)
  {
    ETH_MODE = true;
    Serial.print("connecting ethernet");
    while (!eth.connected())
    {
      Serial.print(".");
      TIMEOUT_ETHERNET++;
      if (TIMEOUT_ETHERNET > 20)
      {
        ETH_MODE = false;
        Serial.println("Ethernet Tidak Terhubung !!");
        Serial.println("Mencoba Menghubungkan Melalui WiFi ...");
        break;
      }
      delay(1000);
    }
    if (ETH_MODE)
    {
      Serial.println();
      Serial.print("Ethernet ip address: ");
      Serial.println(eth.localIP());
      Serial.print("Ethernet subnetMask: ");
      Serial.println(eth.subnetMask());
      Serial.print("Ethernet gateway: ");
      Serial.println(eth.gatewayIP());
      client.setServer(mqtt_server, 1883);
      client.setCallback(callback);
      if (!client.connected())
      {
        reconnect();
      }
    }
  }
  else
  {
    Serial.println("Ethernet Tidak Terhubung !!");
    Serial.println("Mencoba Menghubungkan Melalui WiFi ...");
  }

  if (!ETH_MODE)
  {
    WiFi.config(IP, GATEWAY, NETMASK, DNS);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.println("Connecting To Network ...");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WIFI_ON_SETUP = true;
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    reconnect();
  }

  // wifiServer.on("/", webpage);
  wifiServer.begin();

  wifiServer.on("/", handleRoot);
  wifiServer.on("/login", handleLogin);
  wifiServer.on("/inline", []()
                { wifiServer.send(200, "text/plain", "this works without need of authentication"); });

  wifiServer.onNotFound(handleNotFound);
  // ask server to track these headers
  wifiServer.collectHeaders("User-Agent", "Cookie");
  wifiServer.begin();
  Serial.println("HTTP server started");

  WIFI_ON_SETUP = false;
  // ========================================== AKHIR INISIASI SERIAL ========================================== //

  // ============================================= INISIASI TIMER ============================================= //
  timer_serial.start();
  timer_proses.start();
  // ========================================== AKHIR INISIASI TIMER ========================================== //

  // ============================================= PEMBACAAN MEMORY ============================================= //

  // ========================================== AKHIR PEMBACAAN MEMORY ========================================== //
}

void loop()
{
  timer_serial.update();
  timer_proses.update();
}