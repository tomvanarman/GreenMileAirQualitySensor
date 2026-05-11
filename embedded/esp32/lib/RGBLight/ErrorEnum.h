// ErrorEnum.h
#ifndef ERROR_ENUM_H
#define ERROR_ENUM_H

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

#endif // ERROR_ENUM_H
