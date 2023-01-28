#include <SmingCore.h>
#include <AppSettings.h>
#include <JsonObjectStream.h>
#include <planned_servo.h>
#include <servo.h>
#include <motor_composite.h>
#include <esp8266_pins.h>
#include <state.h>

HttpServer server;
PlannedServo<FeederServo> gripper_servo{0.5, 1.0, D5, 1000, 2000};
PlannedServo<FeederServo> lifter_servo{1.0, 0.5, D6, 800, 2200};
PlannedServo<Motors> motors{0.5, 1, D0, D1, 1000, 2000};

SimpleTimer main_loop_timer;

constexpr float main_loop_interval = 0.02;				 // sec
float servo_loop_time = 0;								 // sec
float ball_release_to_push_delay = 0.5;					 // sec
constexpr float start_grabbing_to_lift_down_delay = 0.5; // sec

bool AreActuatorsReady()
{
	return gripper_servo.IsReady() && lifter_servo.IsReady();
}

void TransitionTo(State state)
{
	g_state = state;
	if((state == State::Feeding) || (state == State::WarmUp)) {
		servo_loop_time = 0;
	}
}

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

		AppSettings.get<GripperServoStartPosition>() =
			request.getPostParameter(GripperServoStartPosition::name).toFloat();
		AppSettings.get<GripperServoEndPosition>() = request.getPostParameter(GripperServoEndPosition::name).toFloat();

		AppSettings.get<LifterServoStartPosition>() =
			request.getPostParameter(LifterServoStartPosition::name).toFloat();
		AppSettings.get<LifterServoEndPosition>() = request.getPostParameter(LifterServoEndPosition::name).toFloat();

		AppSettings.get<BallReleaseToPushTimeDelay>() =
			request.getPostParameter("ball_release_to_push_time_delay").toFloat();
		AppSettings.save();
		Serial.println(_F("save new dev config"));

		motors.SetPwmOffsets({AppSettings.get<LeftMotorOffset>(), AppSettings.get<RightMotorOffset>()});
		gripper_servo.InitByStartAndEnd(AppSettings.get<GripperServoStartPosition>(),
										AppSettings.get<GripperServoEndPosition>());
		lifter_servo.InitByStartAndEnd(AppSettings.get<LifterServoStartPosition>(),
									   AppSettings.get<LifterServoEndPosition>());

		ball_release_to_push_delay = AppSettings.get<BallReleaseToPushTimeDelay>();
		TransitionTo(State::Stop);
	}
}

void onApplyConfig(HttpRequest& request, HttpResponse& response)
{
	if(request.method == HTTP_POST) {
		AppSettings.get<ShootingPower>() = request.getPostParameter("power").toInt();
		AppSettings.get<ShootingIntervalSec>() = request.getPostParameter("interval").toFloat();
		AppSettings.save();
		Serial.println(_F("save new config"));

		float shooting_interval_sec = AppSettings.get<ShootingIntervalSec>();
		gripper_servo.InitByDuration(shooting_interval_sec);
		lifter_servo.InitByDuration(shooting_interval_sec);
		motors.Init(shooting_interval_sec, 0, AppSettings.get<ShootingPower>(), false);

		TransitionTo(State::Stop);
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
		lifter_servo.Write(servo_pwm);
	}
}

void onStartFeeding(HttpRequest& request, HttpResponse& response)
{
	TransitionTo(State::WarmUp);
}

void onStopFeeding(HttpRequest& request, HttpResponse& response)
{
	TransitionTo(State::Stop);
}

void loadConfig();

void feeding_loop()
{
	gripper_servo.Plan(servo_loop_time);

	const auto lift_t = servo_loop_time - start_grabbing_to_lift_down_delay;
	lifter_servo.Plan(lift_t);

	const auto motor_t = lift_t - ball_release_to_push_delay;
	motors.Plan(motor_t);

	servo_loop_time += main_loop_interval;
}

void warm_up_motors()
{
	const auto lift_t = servo_loop_time - start_grabbing_to_lift_down_delay;
	const auto motor_t = lift_t - ball_release_to_push_delay;
	motors.Plan(motor_t);
	servo_loop_time += main_loop_interval;

	float shooting_interval_sec = AppSettings.get<ShootingIntervalSec>();
	if(servo_loop_time >= shooting_interval_sec) {
		TransitionTo(State::Feeding);
	}
}

void main_loop()
{
	switch(g_state) {
	case State::Init:
		motors.Stop();
		loadConfig();
		TransitionTo(State::Smoothing);
		break;
	case State::Stop:
		motors.Stop();
		lifter_servo.InitSmoothen();
		gripper_servo.InitSmoothen();
		TransitionTo(State::Smoothing);
		break;
	case State::Smoothing:
		if(AreActuatorsReady()) {
			TransitionTo(State::Ready);
		}
		break;

	case State::Ready:
		//do nothing
		break;

	case State::WarmUp:
		warm_up_motors();
		break;

	case State::Feeding:
		feeding_loop();
		break;
	}
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

void loadConfig()
{
	AppSettings.load();
	motors.SetPwmOffsets({AppSettings.get<LeftMotorOffset>(), AppSettings.get<RightMotorOffset>()});

	float shooting_interval_sec = constrain(AppSettings.get<ShootingIntervalSec>().value, 1, 10);

	gripper_servo.Init(shooting_interval_sec, AppSettings.get<GripperServoStartPosition>().value,
					   AppSettings.get<GripperServoEndPosition>().value);

	ball_release_to_push_delay = AppSettings.get<BallReleaseToPushTimeDelay>();
	lifter_servo.Init(shooting_interval_sec, AppSettings.get<LifterServoStartPosition>().value,
					  AppSettings.get<LifterServoEndPosition>().value);

	uint16_t shooting_power = constrain(AppSettings.get<ShootingPower>().value, 0, 200);
	motors.Init(shooting_interval_sec, 0, shooting_power, false);
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
