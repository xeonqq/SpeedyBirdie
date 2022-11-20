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

	JsonObject AsJsonObject()
	{
		DynamicJsonDocument doc(std::tuple_size_v<EEPROMData> * 30);
		if(Json::loadFromFile(doc, APP_SETTINGS_FILE)) {
			return doc.as<JsonObject>();
		}
		return JsonObject{};
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

		Json::saveToFile(doc, APP_SETTINGS_FILE);
	}

	bool exist()
	{
		return fileExist(APP_SETTINGS_FILE);
	}
};

static ApplicationSettingsStorage AppSettings;

#endif /* INCLUDE_APPSETTINGS_H_ */
