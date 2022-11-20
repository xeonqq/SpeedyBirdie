#include <SmingCore.h>

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

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
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

	startWebServer();

	// Run our method when station was connected to AP
}
