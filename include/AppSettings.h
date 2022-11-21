/*
 * AppSettings.h
 *
 *  Created on: 13 ??? 2015 ?.
 *      Author: Anakod
 */

#include <SmingCore.h>
#include <ArduinoJson.h>
#include <eeprom_decorator.h>

#ifndef INCLUDE_APPSETTINGS_H_
#define INCLUDE_APPSETTINGS_H_

#define APP_SETTINGS_FILE "settings.json" // leading point for security reasons :)

struct ApplicationSettingsStorage {
	EEPROMData settings;

	ApplicationSettingsStorage()
	{
		if(exist()) {
			load();
		}
	}

	void ramObjectToJsonObject(JsonObject& json)
	{
		for_each_in_tuple(settings, [&json](const auto& data) { json[data.name] = data.value; });
	}

	JsonObject AsJsonObject()
	{
		DynamicJsonDocument doc(std::tuple_size_v<EEPROMData> * 30);
		if(Json::loadFromFile(doc, APP_SETTINGS_FILE)) {
			return doc.as<JsonObject>();
		}
		return JsonObject{};
	}

	template <typename T> T& get()
	{
		return std::get<T>(settings);
	}

	void load()
	{
		DynamicJsonDocument doc(std::tuple_size_v<EEPROMData> * 30);
		if(Json::loadFromFile(doc, APP_SETTINGS_FILE)) {
			for_each_in_tuple(settings, [&doc](auto& data) { data.value = doc[data.name]; });
		}
	}

	void save()
	{
		DynamicJsonDocument doc(std::tuple_size_v<EEPROMData> * 30);

		for_each_in_tuple(settings, [&doc](const auto& data) { doc[data.name] = data.value; });

		bool success = Json::saveToFile(doc, APP_SETTINGS_FILE);

		Serial << "save to file:" << success;
		Serial.println();
	}

	bool exist()
	{
		return fileExist(APP_SETTINGS_FILE);
	}
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
