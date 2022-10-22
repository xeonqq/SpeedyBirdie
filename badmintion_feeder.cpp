#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <AccelStepper.h>
#include <ESP_EEPROM.h>
#include <Servo.h>

#include "brushless_motor.h"
#include "config.h"
#include "eeprom_decorator.h"

#define IN1 1
#define IN2 3
#define IN3 15
#define IN4 13

StepperConfig stepper_config{200};
BadmintonFeederConfig badmintion_feeder_config{5};

Servo servo;

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN2, IN3, IN4);
BrushlessMotor motor1{D0};
BrushlessMotor motor2{D1};

AsyncWebServer server(80);
const char *ssid = "BirdyFeeder";
const char *password = "svlohhof";
IPAddress apIP(192, 168, 0, 1);
EEPROMDecorator eeprom;

void handleNotFound(AsyncWebServerRequest *request) {
  String path = request->url();
  if (!SPIFFS.exists(path)) {
    request->send(404);
    return;
  }
  String contentType = "text/plain";
  if (path.endsWith(".css")) {
    contentType = "text/css";
  } else if (path.endsWith(".html")) {
    contentType = "text/html";
  } else if (path.endsWith(".js")) {
    contentType = "application/javascript";
  }
  request->send(SPIFFS, path, contentType);
}

void ConfigureServer(AsyncWebServer &server) {
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Send index.html.");
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/eeprom.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    const auto json_str = ToJsonBuf(eeprom.ReadData());
    request->send(200, "application/json", json_str);
  });

  server.on("/apply_config", HTTP_POST, [](AsyncWebServerRequest *request) {
    String shoot_power = request->arg("power");              // 0-1000
    String shooting_interval_sec = request->arg("interval"); // 0-8
    Serial.println("Current shoot power: " + shoot_power);
    Serial.println("Current shoot interval: " + shooting_interval_sec);
    /*motor1.RunSpeed(shoot_power.toFloat());*/
    eeprom.Write<ShootingPower>(shoot_power.toInt());
    eeprom.Write<ShootingIntervalSec>(shooting_interval_sec.toInt());
    request->send(200);
  });

  server.onNotFound(handleNotFound);
  // Send Favicon
  server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  // Begin Server
  server.begin();
}

void SetupSoftAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  ConfigureServer(server);
}

void setup() {
  Serial.begin(115200);
  eeprom.Init();
  SetupSoftAP();

  motor1.Calibrate();
  motor2.Calibrate();

  servo.attach(D4);
  // set the speed and acceleration
  const auto speed = stepper_config.GetSpeedStepsPerSec(
      badmintion_feeder_config.GetShootIntervalSec());
  stepper.setMaxSpeed(speed * 2);
  stepper.setSpeed(speed);
  stepper.setAcceleration(200);
}

void loop() {

  stepper.move(stepper_config.GetStepsPerRevolution() * 4);
  stepper.run();
  // const auto data = eeprom.ReadData();
  // Serial.println(std::get<ShootingPower>(data).value);
  // const auto data = eeprom.Read<ShootingPower>();
  // Serial.println(data.value);

  /*motor1.RunSpeed(0.0);*/
  delay(100);
}
