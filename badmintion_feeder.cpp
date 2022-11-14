#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ESP_EEPROM.h>
#include <MicroQt.h>

#include "EventLoop.h"
#include "Server.h"
#include "eeprom_decorator.h"
#include "motor_composite.h"
#include "planned_servo.h"

Motors motors{D0, D1};

AsyncWebServer server(80);
const char *ssid = "BirdyFeeder";
const char *password = "chinasprung";
IPAddress apIP(192, 168, 0, 1);

PlannedServo planned_servo{D6, 0.5, 1000, 2000};
PlannedServo ball_release_servo{D5, 0.5, 1000, 2000};

EEPROMDecorator eeprom;

const float servo_loop_interval = 0.02; // sec
float servo_loop_time = 0;              // sec
float ball_release_to_push_delay = 0.5; // sec
MicroQt::Timer servo_timer{static_cast<uint32_t>(servo_loop_interval * 1000)};

void onApplyConfigRequest(uint16_t shoot_power, float shooting_interval_sec) {
  eeprom.Write<ShootingPower>(shoot_power);
  eeprom.Write<ShootingIntervalSec>(shooting_interval_sec);
  planned_servo.InitByDuration(shooting_interval_sec);
  ball_release_servo.InitByDuration(shooting_interval_sec);
};

void onApplyDevConfigRequest(uint16_t left_motor_offset,
                             uint16_t right_motor_offset,
                             float servo_end_position,
                             float ball_release_servo_start_position,
                             float ball_release_to_push_time_delay) {
  eeprom.Write<LeftMotorOffset>(left_motor_offset);
  eeprom.Write<RightMotorOffset>(right_motor_offset);
  eeprom.Write<ServoEndPosition>(servo_end_position);
  eeprom.Write<BallReleaseServoStartPosition>(
      ball_release_servo_start_position);
  eeprom.Write<BallReleaseToPushTimeDelay>(ball_release_to_push_time_delay);
  motors.SetPwmOffsets({left_motor_offset, right_motor_offset});
  planned_servo.InitByStartAndEnd(0, servo_end_position);
  ball_release_servo.InitByStartAndEnd(
      ball_release_servo_start_position,
      FeederServo::GetNetualPositionPercentage());
  ball_release_to_push_delay = ball_release_to_push_time_delay;
};

void onStartFeeding() {
  auto shoot_power = eeprom.Read<ShootingPower>();
  motors.RunSpeed(shoot_power);
  servo_timer.start();
}

void onStopFeeding() {
  motors.Stop();
  servo_timer.stop();
  servo_loop_time = 0;
  ball_release_servo.InitSmoothen();
  planned_servo.InitSmoothen();
}

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

  server.on("/dev", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Send dev.html.");
    request->send(SPIFFS, "/dev.html", "text/html");
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
    MicroQt::eventLoop.enqueueEvent([=]() {
      onApplyConfigRequest(shoot_power.toInt(),
                           shooting_interval_sec.toFloat());
    });
    request->send(200);
  });

  server.on("/apply_dev_config", HTTP_POST, [](AsyncWebServerRequest *request) {
    String left_motor_offset = request->arg("left_motor_offset");   // 0-1000
    String right_motor_offset = request->arg("right_motor_offset"); // 0-1000
    String servo_final_position = request->arg("servo_final_position"); // 0-1
    String ball_release_servo_final_position =
        request->arg("ball_release_servo_final_position"); // 0-1
    String ball_release_to_push_time_delay =
        request->arg("ball_release_to_push_time_delay"); // 0-2
    Serial.println("servo final position: " + servo_final_position);
    MicroQt::eventLoop.enqueueEvent([=]() {
      onApplyDevConfigRequest(left_motor_offset.toInt(),
                              right_motor_offset.toInt(),
                              servo_final_position.toFloat(),
                              ball_release_servo_final_position.toFloat(),
                              ball_release_to_push_time_delay.toFloat());
    });
    request->send(200);
  });

  server.on("/left_pwm", HTTP_POST, [](AsyncWebServerRequest *request) {
    String left_pwm = request->arg("left_pwm"); // 0-1000
    MicroQt::eventLoop.enqueueEvent(
        [&motors, left_pwm]() { motors.RunSpeedRaw(0, left_pwm.toInt()); });
    request->send(200);
  });

  server.on("/right_pwm", HTTP_POST, [](AsyncWebServerRequest *request) {
    String right_pwm = request->arg("right_pwm"); // 0-1000
    MicroQt::eventLoop.enqueueEvent(
        [&motors, right_pwm]() { motors.RunSpeedRaw(1, right_pwm.toInt()); });
    request->send(200);
  });

  server.on("/servo_pwm", HTTP_POST, [](AsyncWebServerRequest *request) {
    String servo_pwm = request->arg("servo_pwm"); // 0-1
    MicroQt::eventLoop.enqueueEvent([&ball_release_servo, servo_pwm]() {
      ball_release_servo.Write(servo_pwm.toFloat());
    });
    request->send(200);
  });

  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request) {
    MicroQt::eventLoop.enqueueEvent(onStartFeeding);
    request->send(200);
  });

  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest *request) {
    MicroQt::eventLoop.enqueueEvent(onStopFeeding);
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

auto servo_loop = [&servo_loop_time, &planned_servo, &ball_release_servo,
                   &servo_loop_interval, &ball_release_to_push_delay]() {
  ball_release_servo.Plan(servo_loop_time);
  planned_servo.Plan(servo_loop_time - ball_release_to_push_delay);
  servo_loop_time += servo_loop_interval;
};

MicroQt::Timer timer_next_event;

void setup() {
  Serial.begin(115200);
  eeprom.Init();
  SetupSoftAP();

  MicroQt::eventLoop.enqueueEvent([&motors]() { motors.Stop(); });
  const auto left_motor_offset = eeprom.Read<LeftMotorOffset>();
  const auto right_motor_offset = eeprom.Read<RightMotorOffset>();
  motors.SetPwmOffsets({left_motor_offset, right_motor_offset});

  float shooting_interval_sec = eeprom.Read<ShootingIntervalSec>();
  float servo_end_position = eeprom.Read<ServoEndPosition>();
  shooting_interval_sec = constrain(shooting_interval_sec, 1, 10);
  servo_end_position = constrain(servo_end_position, 0, 1);

  planned_servo.Init(shooting_interval_sec, 0, servo_end_position);

  float ball_release_servo_start_position =
      eeprom.Read<BallReleaseServoStartPosition>();
  ball_release_servo_start_position =
      constrain(ball_release_servo_start_position, 0, 1);
  ball_release_servo.Init(shooting_interval_sec,
                          ball_release_servo_start_position,
                          FeederServo::GetNetualPositionPercentage());

  ball_release_to_push_delay = eeprom.Read<BallReleaseToPushTimeDelay>();

  servo_timer.sglTimeout.connect(servo_loop);

  /*  timer_next_event.sglTimeout.connect(*/
  /*[&servo_timer]() { servo_timer.start(); });*/
  /*timer_next_event.setSingleShot(true);*/
  /*timer_next_event.start(ball_release_servo.GetPushingTime());*/
}

void loop() { MicroQt::eventLoop.exec(); }
