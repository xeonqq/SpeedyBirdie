#include <SmingCore.h>
#include <AppSettings.h>
#include <JsonObjectStream.h>
#include <planned_servo.h>
#include <servo.h>
#include <motor_composite.h>
#include <esp8266_pins.h>

HttpServer server;
PlannedServo<FeederServo> pushing_servo{0.5, D6, 1000, 2000};
PlannedServo<FeederServo> ball_release_servo{0.5, D5, 1000, 2000};
PlannedServo<Motors> motors{0.5, D0, D1, 1000, 2000};

SimpleTimer main_loop_timer;

const float main_loop_interval = 0.02;  // sec
float servo_loop_time = 0;				// sec
float ball_release_to_push_delay = 0.5; // sec

void onIndex(HttpRequest& request, HttpResponse& response)
{
	auto tmpl = new TemplateFileStream(F("index.html"));
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();
	if(file != APP_SETTINGS_FILE) {
		response.setCache(86400, true); // problematic when sending modifiable json
	}
	response.sendFile(file);
}

void onApplyDevConfig(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		AppSettings.get<LeftMotorOffset>() = request.getPostParameter("left_motor_offset").toInt();
		AppSettings.get<RightMotorOffset>() = request.getPostParameter("right_motor_offset").toInt();
		AppSettings.get<ServoEndPosition>() = request.getPostParameter("servo_final_position").toFloat();
		AppSettings.get<BallReleaseServoStartPosition>() =
			request.getPostParameter("ball_release_servo_final_position").toFloat();
		AppSettings.get<BallReleaseToPushTimeDelay>() =
			request.getPostParameter("ball_release_to_push_time_delay").toFloat();
		AppSettings.save();
		Serial.println(_F("save new dev config"));
	}
}

void onApplyConfig(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		AppSettings.get<ShootingPower>() = request.getPostParameter("power").toInt();
		AppSettings.get<ShootingIntervalSec>() = request.getPostParameter("interval").toFloat();
		AppSettings.save();
		Serial.println(_F("save new config"));
	}
}

void onLeftPwm(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		const auto left_pwm = request.getPostParameter("left_pwm").toInt();
		motors.WriteRaw(0, left_pwm);
	}
}

void onRightPwm(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		const auto right_pwm = request.getPostParameter("right_pwm").toInt();
		motors.WriteRaw(1, right_pwm);
	}
}

void onServoPwm(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		const auto servo_pwm = request.getPostParameter("servo_pwm").toFloat();
		ball_release_servo.Write(servo_pwm);
	}
}

void onStartFeeding(HttpRequest& request, HttpResponse& response)
{
	main_loop_timer.start();
}

void onStopFeeding(HttpRequest& request, HttpResponse& response)
{
	motors.Stop();
	main_loop_timer.stop();
	servo_loop_time = 0;
	ball_release_servo.InitSmoothen();
	pushing_servo.InitSmoothen();
}

void main_loop()
{
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/apply_dev_config", onApplyDevConfig);
	server.paths.set("/apply_config", onApplyConfig);
	server.paths.set("/left_pwm", onLeftPwm);
	server.paths.set("/right_pwm", onRightPwm);
	server.paths.set("/servo_pwm", onServoPwm);
	server.paths.set("/start", onStartFeeding);
	server.paths.set("/stop", onStopFeeding);

	server.paths.setDefault(onFile);

	Serial.println(_F("\r\n"
					  "=== WEB SERVER STARTED ==="));
	Serial.println(_F("==========================\r\n"));
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// Soft access point
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config(_F("BirdyFeeder"), "chinasprung", AUTH_WPA_WPA2_PSK);

	// Optional: Change IP addresses (and disable DHCP)
	WifiAccessPoint.setIP(IpAddress(192, 168, 0, 1));

	if(!AppSettings.exist()) {
		AppSettings.save();
	}

	startWebServer();

	main_loop_timer.initializeMs(static_cast<int>(main_loop_interval * 1000), main_loop).start();

	// Run our method when station was connected to AP
}
