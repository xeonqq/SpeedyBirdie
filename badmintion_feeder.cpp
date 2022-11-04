#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <AccelStepper.h>
#include <ESP_EEPROM.h>
#include <MicroQt.h>

#include "EventLoop.h"
#include "Server.h"
#include "config.h"
#include "eeprom_decorator.h"
#include "minimum_jerk_trajectory_planner.h"
#include "motor_composite.h"
#include "servo.h"

#define IN1 1
#define IN2 3
#define IN3 15
#define IN4 13

StepperConfig stepper_config{200};

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN2, IN3, IN4);
Motors motors{D0, D1};

AsyncWebServer server(80);
const char *ssid = "BirdyFeeder";
const char *password = "chinasprung";
IPAddress apIP(192, 168, 0, 1);
FeederServo servo(D2);

EEPROMDecorator eeprom;

const float servo_loop_interval = 0.02;        // sec
const float servo_end_position_duration = 3.0; // sec;
MicroQt::Timer servo_timer{static_cast<uint32_t>(servo_loop_interval * 1000)};

const float stepper_loop_interval = 0.02; // sec
MicroQt::Timer stepper_timer{
    static_cast<uint32_t>(stepper_loop_interval * 1000)};

MinimumJerkTrajortoryPlanner planner{};

void adaptStepperSpeed(float shooting_interval_sec) {
  const auto speed = stepper_config.GetSpeedStepsPerSec(shooting_interval_sec);
  stepper.setMaxSpeed(speed * 2);
  stepper.setSpeed(speed);
}

void onApplyConfigRequest(uint16_t shoot_power, float shooting_interval_sec) {
  eeprom.Write<ShootingPower>(shoot_power);
  eeprom.Write<ShootingIntervalSec>(shooting_interval_sec);
  planner.InitByDuration(shooting_interval_sec / 2);
  adaptStepperSpeed(shooting_interval_sec);

  motors.RunSpeed(shoot_power);
};

void onApplyDevConfigRequest(uint16_t left_motor_offset,
                             uint16_t right_motor_offset,
                             float servo_end_position) {
  eeprom.Write<LeftMotorOffset>(left_motor_offset);
  eeprom.Write<RightMotorOffset>(right_motor_offset);
  eeprom.Write<ServoEndPosition>(servo_end_position);
  motors.SetPwmOffsets({left_motor_offset, right_motor_offset});
  planner.InitByEndPosition(servo_end_position);
};

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
    Serial.println("servo final position: " + servo_final_position);
    MicroQt::eventLoop.enqueueEvent([=]() {
      onApplyDevConfigRequest(left_motor_offset.toInt(),
                              right_motor_offset.toInt(),
                              servo_final_position.toFloat());
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

auto servo_loop = [&planner, &servo, &servo_loop_interval]() {
  static float t = 0;
  static int direction = 1;

  const auto new_position = planner.Plan(t);
#ifdef DEBUG_SERVO
  Serial.print(" @");
  Serial.print(t);
  Serial.print(" p: ");
  Serial.println(new_position);
#endif
  servo.Write(new_position);
  const float eps = std::numeric_limits<float>::epsilon();
  if ((t + servo_loop_interval) > (planner.GetDuration() + eps)) {
    direction = -1;
    t = planner.GetDuration() - servo_loop_interval;
  } else if ((t - servo_loop_interval) < -eps) {
    direction = 1;
    t = servo_loop_interval;
  } else {
    t += servo_loop_interval * direction;
  }
};

void setup() {
  Serial.begin(115200);
  eeprom.Init();
  SetupSoftAP();

  motors.Init();
  const auto left_motor_offset = eeprom.Read<LeftMotorOffset>();
  const auto right_motor_offset = eeprom.Read<RightMotorOffset>();
  motors.SetPwmOffsets({left_motor_offset, right_motor_offset});

  float shooting_interval_sec = eeprom.Read<ShootingIntervalSec>();
  float servo_end_position = eeprom.Read<ServoEndPosition>();
  shooting_interval_sec = constrain(shooting_interval_sec, 0, 10);
  servo_end_position = constrain(servo_end_position, 0, 1);
  planner.Init(shooting_interval_sec / 2, servo_end_position);

  servo.Reset();
  servo_timer.sglTimeout.connect(servo_loop);
  servo_timer.start();

  // set the speed and acceleration
  adaptStepperSpeed(shooting_interval_sec);
  stepper.setAcceleration(200);

  stepper_timer.sglTimeout.connect([&stepper_config, &stepper]() {
    stepper.move(stepper_config.GetStepsPerRevolution() * 4);
    stepper.run();
  });
  stepper_timer.start();
}

void loop() { MicroQt::eventLoop.exec(); }
