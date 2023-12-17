#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <DHT.h>
#include <Arduino_JSON.h>
#include "SPIFFS.h"
#include <WebSocketsServer.h>

const char *ssid = "TP-BamBo";
const char *password = "123456789";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "vn.pool.ntp.org");

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const int DHTPIN = 21;
const int DHTTYPE = DHT11;
const int LIGHTPIN = 17;

DHT dht(DHTPIN, DHTTYPE);
void setup()
{
  Serial.begin(115200);
  dht.begin();
  pinMode(34, INPUT); // Tín hiệu vào từ cảm biến
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  initFS();
  initWiFi();
  initWebSocket();
  // Bắt đầu dịch vụ NTP
  timeClient.begin();

  // Đặt múi giờ cho Việt Nam (UTC+7)
  timeClient.setTimeOffset(7 * 3600);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}

// Biến JSON để lưu trữ dữ liệu độ ẩm và nhiệt độ từ cảm biến DHT11
JSONVar sensorData;
float temperature;
float humidity;
float doc_cb;
float soil;
String pump_status = "";
String state;
float setMin = 10;
float setMax = 35;
int sethours;
int setminute;
int mess;
float light;
String getSensorData()
{
  doc_cb = analogRead(34);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  //  temperature = random(50);
  //  humidity = random(100);
  soil = 100 - map(doc_cb, 0, 4095, 0, 100);
  light = digitalRead(LIGHTPIN);
  // Lưu dữ liệu từ cảm biến DHT11 vào biến JSON
  sensorData["light"] = light;
  sensorData["temperature"] = temperature;
  sensorData["humidity"] = humidity;
  sensorData["soil"] = soil;
  sensorData["pump_status"] = pump_status;
  //  sensorData["setMin"]=setMin;
  //  sensorData["setMax"]=setMax;

  // Chuyển đổi JSON thành chuỗi
  String jsonString = JSON.stringify(sensorData);
  return jsonString;
}

void notifyClients(String data)
{
  ws.textAll(data);
}

void initFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    String message = (char *)data;
    Serial.print("Received WebSocket message: ");
    Serial.println(message);
    if (strcmp((char *)data, "getValues") == 0)
    {
      notifyClients(getSensorData());
    }
    if (message.indexOf("setRelayState") != -1)
    {
      // Phân tích thông điệp và kiểm tra trạng thái
      int index = message.indexOf("state") + 8;
      state = message.substring(index + 1, index + 3);
      Serial.println(state);
      mess = 0;
    }
    else if (message.indexOf("setMinMaxValues") != -1)
    {
      // Phân tích thông điệp và kiểm tra giá trị min và max
      int minIndex = message.indexOf("min") + 7;
      int maxIndex = message.indexOf("max") + 7;
      float receivedMin = message.substring(minIndex, minIndex + 2).toFloat();
      float receivedMax = message.substring(maxIndex, maxIndex + 2).toFloat();

      // Gán giá trị nhận được vào biến min và max
      setMin = receivedMin;
      setMax = receivedMax;

      // In giá trị min và max ra Serial Monitor
      Serial.print("Received and updated min value: ");
      Serial.println(setMin);
      Serial.print("Received and updated max value: ");
      Serial.println(setMax);
      mess = 1;
    }
    else if (message.indexOf("setTimerValues") != -1)
    {
      int minIndex = message.indexOf("min") + 7;
      int maxIndex = message.indexOf("max") + 7;
      int hoursIndex = message.indexOf("hours") + 9;
      int minuteIndex = message.indexOf("minutes") + 11;

      float receivedMin = message.substring(minIndex, minIndex + 2).toFloat();
      float receivedMax = message.substring(maxIndex, maxIndex + 2).toFloat();
      int receivedHours = message.substring(hoursIndex, hoursIndex + 2).toInt();
      int receivedMinute = message.substring(minuteIndex, minuteIndex + 2).toInt();

      // Gán giá trị nhận được vào biến min và max
      setMin = receivedMin;
      setMax = receivedMax;
      sethours = receivedHours;
      setminute = receivedMinute;
      // In giá trị min và max ra Serial Monitor
      Serial.print("Received and updated min, max value: ");
      Serial.println(setMin);
      Serial.println(setMax);
      Serial.print("Received and updated hours minute value: ");
      Serial.println(sethours);
      Serial.println(setminute);
      mess = 2;
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void getPumpStatus()
{
  if (digitalRead(22) == LOW)
  {
    pump_status = "ON";
  }
  else
  {
    pump_status = "OFF";
  }
}
void settingpump()
{
  if (state == "ON")
  {
    digitalWrite(22, LOW);
    pump_status = "ON";
  }
  else if (state == "OF")
  {
    digitalWrite(22, HIGH);
    pump_status = "OFF";
  }
  else if (state == "LF")
  {
    digitalWrite(23, LOW);
  }
  else if (state == "LN")
  {
    digitalWrite(23, HIGH);
  }
}

void pump()
{
  if (soil < setMin)
  {
    digitalWrite(22, LOW);
    pump_status = "ON";
  }
  else if (soil >= setMax)
  {
    digitalWrite(22, HIGH);
    pump_status = "OFF";
  }
}
int testt = 0;
void setTimerPump()
{
  if (soil < setMax)
  {
    digitalWrite(22, LOW);
    pump_status = "ON";
  }
}
void setTimer()
{
  int hours1 = timeClient.getHours();
  int minutes1 = timeClient.getMinutes();
  if (hours1 == sethours && minutes1 == setminute)
  {
    setTimerPump();
    testt = 1;
  }
}
void loop()
{
  ws.cleanupClients();

  getPumpStatus();
  if (mess == 0)
  {
    settingpump();
  }
  else if (mess == 1)
  {
    pump();
  }
  else if (mess == 2)
  {
    setTimer();
  }
  if (testt == 1)
  {
    if (soil >= setMax)
    {
      digitalWrite(22, HIGH);
      pump_status = "OFF";
    }
  }
  String sensorData = getSensorData();
  notifyClients(sensorData);
  timeClient.update();

  // Lấy thời gian hiện tại
  Serial.println(timeClient.getFormattedTime());
  delay(1000);
}
