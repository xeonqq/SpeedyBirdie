#include <SmingCore.h>
#include <AppSettings.h>
#include <JsonObjectStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
HttpServer server;

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

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/apply_dev_config", onApplyDevConfig);
	server.paths.set("/apply_config", onApplyConfig);
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

	// Run our method when station was connected to AP
}
