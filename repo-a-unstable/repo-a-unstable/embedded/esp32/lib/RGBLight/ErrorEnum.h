// ErrorEnum.h
#ifndef ERROR_ENUM_H
#define ERROR_ENUM_H

#include <Arduino.h>

// Enum for possible setup errors in the sensor box
enum class SetupError {
	NONE = 0,                  // No error
	WIFI_INIT_FAILED,          // WiFi initialization failed
	SENSOR_NOT_FOUND,          // Sensor not detected
	SENSOR_INIT_FAILED,        // Sensor initialization failed
	MQTT_INIT_FAILED,          // MQTT client setup failed
	HTTP_INIT_FAILED,          // HTTP client setup failed
	SD_CARD_INIT_FAILED,       // SD card initialization failed
	CONFIG_LOAD_FAILED,        // Configuration file could not be loaded
	INVALID_CREDENTIALS,       // Invalid credentials (WiFi, MQTT, etc.)
	TIME_SYNC_FAILED,          // Time synchronization failed
	UNKNOWN_ERROR              // Unknown error
};

// Helper to convert SetupError to an Arduino String
inline String ToString(SetupError e)
{
	switch (e)
	{
	case SetupError::NONE:
		return "NONE";
	case SetupError::WIFI_INIT_FAILED:
		return "WIFI_INIT_FAILED";
	case SetupError::SENSOR_NOT_FOUND:
		return "SENSOR_NOT_FOUND";
	case SetupError::SENSOR_INIT_FAILED:
		return "SENSOR_INIT_FAILED";
	case SetupError::MQTT_INIT_FAILED:
		return "MQTT_INIT_FAILED";
	case SetupError::HTTP_INIT_FAILED:
		return "HTTP_INIT_FAILED";
	case SetupError::SD_CARD_INIT_FAILED:
		return "SD_CARD_INIT_FAILED";
	case SetupError::CONFIG_LOAD_FAILED:
		return "CONFIG_LOAD_FAILED";
	case SetupError::INVALID_CREDENTIALS:
		return "INVALID_CREDENTIALS";
	case SetupError::TIME_SYNC_FAILED:
		return "TIME_SYNC_FAILED";
	case SetupError::UNKNOWN_ERROR:
	default:
		return "UNKNOWN_ERROR";
	}
}

#endif // ERROR_ENUM_H
