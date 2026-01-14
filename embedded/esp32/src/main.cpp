// // External libraries
// #include <Arduino.h>
// #include <Wire.h>
// #include <deque>
// #include <algorithm>

// // Custom structured debug logging
// #include "DEBUG.h"

// // Utility Libraries
// #include <JsonBuilder.h>
// #include "ColorMap.h"

// // Sensor Libraries
// #include "SPS30.h"
// #include "SHT41Sensor.h"

// // Actuator Libraries
// #include "LEDStrip.h"
// #include "SegmentDisplay.h"

// // SIM7080 Modem Library
// #include "SIM7080.h"

// // Network Libraries
// #include "NetworkServer.h"
// #include "CredentialManager.h"
// #include "WifiManager.h"
// #include "HttpManager.h"

// bool useSIM = true;

// // Server configuration for HTTPS POST
// const char *host = "greenmile.tapp.city";
// const char *url = "https://greenmile.tapp.city";
// const char *SPS30path = "/api/data/sps30";
// const char *SHT41path = "/api/data/sht41";

// // Timing constants (in milliseconds)
// constexpr unsigned long kReconnectInterval = 6000;
// constexpr unsigned long kDataTransmissionInterval = 20000;
// constexpr unsigned long kDataMeasurementInterval = 2000;

// // Global objects
// SIM7080 modem("iot.1nce.net"); // APN for 1NCE IoT SIM cards

// CredentialManager credential_manager;
// NetworkServer server(credential_manager);
// WiFiManager network(credential_manager);
// HttpManager httpManager;

// TwoWire WireSensors = TwoWire(1);

// SPS30 sps30;
// SHT41Sensor sht41;

// LEDStrip strip;
// SegmentDisplay segmentDisplay(11, 12, 10); // Data, CLK, CS pins

// // float maxPM = 250.0f;
// // GradientStop stops[] = {
// //     {0.0f / maxPM, 0, 255, 0},      // green
// //     {50.0f / maxPM, 255, 255, 0},   // yellow
// //     {100.0f / maxPM, 255, 165, 0},  // orange
// //     {150.0f / maxPM, 255, 0, 0},    // red
// //     {250.0f / maxPM, 128, 0, 0},  // maroon
// // };

// // For debugging purposes
// float maxPM = 25.0f;
// GradientStop stops[] = {
//     {0.0f / maxPM, 0, 255, 0},    // green
//     {5.0f / maxPM, 255, 255, 0},  // yellow
//     {10.0f / maxPM, 255, 165, 0}, // orange
//     {15.0f / maxPM, 255, 0, 0},   // red
//     {25.0f / maxPM, 128, 0, 0},   // maroon
// };

// ColorMap colorMap(maxPM, stops);

// std::deque<String> postQueue;
// const size_t kMaxQueueSize = 50;

// // NTP server configuration
// const char *ntpServer1 = "149.143.87.22"; // pool.ntp.org
// const char *ntpServer2 = "82.65.248.56";  // europe.pool.ntp.org
// const long gmtOffset_sec = 3600;          // GMT+1
// const int daylightOffset_sec = 0;

// // Function declarations
// void HandleSPS30Logic();
// void HandleSHT41Logic();
// bool trySendQueue(const char *path);
// void initializeTime();
// uint64_t getCurrentTimestampMs();

// void setup()
// {
//   Serial.begin(115200);

//   // Wait for serial port to be ready
//   while (!Serial)
//     ;

//   delay(5000);

//   DEBUG_SECTION("Setup");

//   strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BREATHING);

//   if (useSIM)
//   {
//     //============================================================================
//     // Setup SIM7080 Modem
//     //============================================================================
//     modem.initialize();
//     modem.startModem();

//     segmentDisplay.start();
//     int batteryLevel = modem.getBatteryLevel();
//     segmentDisplay.setBattery(batteryLevel);

//     // modem.setupNetwork();
//     // if (!modem.ensureConnected())
//     // {
//     //   DEBUG_WARN("Failed to connect to network");
//     //   strip.stopLoading();
//     //   strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);
//     //   while (1)
//     //     modem.ensureConnected();
//     // }
//     // strip.stopLoading();
//     // strip.clear();
//   }
//   // else
//   // {
//   //   credential_manager.LoadCredentials();

//   //   if (!credential_manager.ValidateCredentials())
//   //   {
//   //     DEBUG_WARN("Invalid or missing credentials, starting AP for configuration...");
//   //     server.StartAP();

//   //     strip.stopLoading();
//   //     strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

//   //     segmentDisplay.start();
//   //     segmentDisplay.setIPAddress("192.168.4.1");

//   //     // Wait indefinitely in AP mode until configured
//   //     while (true)
//   //     {
//   //       server.HandleRequests();
//   //       delay(10);
//   //     }
//   //   }

//   //   //============================================================================
//   //   // Setup WiFi
//   //   //============================================================================
//   //   network.Connect();

//   //   if (!network.isConnected())
//   //   {
//   //     DEBUG_WARN("Wrong credentials, starting AP for configuration...");
//   //     server.StartAP();

//   //     strip.stopLoading();
//   //     strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

//   //     segmentDisplay.start();
//   //     segmentDisplay.setIPAddress("192.168.4.1");

//   //     // Wait indefinitely in AP mode until configured
//   //     while (true)
//   //     {
//   //       server.HandleRequests();
//   //       delay(10);
//   //     }
//   //   }

//   //   // Initialize NTP time synchronization
//   //   initializeTime();
//   // }

//   // strip.stopLoading();
//   // strip.clear();

//   // // Initialize I2C communication
//   // WireSensors.begin(8, 9);      // SDA, SCL
//   // WireSensors.setClock(100000); // lock to 100 kHz for both SPS30 and SHT41
//   // WireSensors.setTimeOut(100);  // a bit more headroom for long reads

//   // //============================================================================
//   // // Setup SPS30
//   // //============================================================================
//   // strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BREATHING);
//   // if (!sps30.begin(WireSensors))
//   // {
//   //   DEBUG_WARN("Failed to find SPS30 sensor");
//   //   strip.stopLoading();
//   //   strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);
//   //   while (1)
//   //     delay(10);
//   // }
//   // strip.stopLoading();

//   // //============================================================================
//   // // Setup SHT41
//   // //============================================================================
//   // strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BREATHING);
//   // if (!sht41.begin(WireSensors))
//   // {
//   //   DEBUG_WARN("Failed to find SHT41 sensor");
//   //   strip.stopLoading();
//   //   strip.startLoading(CRGB::DarkBlue, LEDStrip::_loadingModeType::BLINKING);
//   //   while (1)
//   //     delay(10);
//   // }
//   // strip.stopLoading();
//   // strip.clear();

//   // // Ensure clock wasn't changed by drivers
//   // WireSensors.setClock(100000);

//   // // Initialize LUT for color mapping
//   // colorMap.InitLUT();

//   // // Initialize segment display
//   // // segmentDisplay.start();
//   // // int batteryLevel = modem.getBatteryLevel();
//   // // segmentDisplay.setBattery(batteryLevel);
//   // delay(3000);
//   // segmentDisplay.clearDisplay();
// }

// void loop()
// {
//   // HandleSHT41Logic();
//   // HandleSPS30Logic();
// }

// void HandleSPS30Logic()
// {
//   unsigned long now = millis();
//   bool can_measure = now - sps30.last_measurement >= kDataMeasurementInterval;
//   bool can_update = now - sps30.last_update >= kDataTransmissionInterval;

//   if (!can_measure)
//     return;

//   SPS30_measurement spsData = sps30.readData();

//   if (!spsData.available)
//   {
//     DEBUG_WARN("Failed to read from sps30 sensor");

//     if (!strip.isLoading())
//       strip.startLoading(CRGB::DeepSkyBlue, LEDStrip::_loadingModeType::BLINKING);

//     return;
//   }

//   if (strip.isLoading())
//     strip.stopLoading();

//   sps30.last_measurement = now;

//   // Update LED strip based on PM2.5 value
//   Color c = colorMap.DataToColor(spsData.mc_2p0);
//   strip.toColor(CRGB(c.r, c.g, c.b), 50);

//   if (!can_update)
//     return;

//   sps30.last_update = now;

//   // Pretty print for debug
//   String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
//                                              {
//           doc["mc_1p0"] = spsData.mc_1p0;
//           doc["mc_2p0"] = spsData.mc_2p0;
//           doc["mc_4p0"] = spsData.mc_4p0;
//           doc["mc_10p0"] = spsData.mc_10p0;
//           doc["nc_0p5"] = spsData.nc_0p5;
//           doc["nc_1p0"] = spsData.nc_1p0;
//           doc["nc_2p5"] = spsData.nc_2p5;
//           doc["nc_4p0"] = spsData.nc_4p0;
//           doc["nc_10p0"] = spsData.nc_10p0;
//           doc["typical_particle_size"] = spsData.typical_particle_size;

//           if (useSIM)
//             doc["time_unix"] = modem.getCurrentTimestampMs() / 1000; 
//           else
//             doc["time_unix"] = getCurrentTimestampMs() / 1000; }, true);

//   DEBUG_BLOCK("Payload");
//   DEBUG_INFO(prettyData);

//   // Compact version for actual POST
//   String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
//                                        {
//           doc["mc_1p0"] = spsData.mc_1p0;
//           doc["mc_2p0"] = spsData.mc_2p0;
//           doc["mc_4p0"] = spsData.mc_4p0;
//           doc["mc_10p0"] = spsData.mc_10p0;
//           doc["nc_0p5"] = spsData.nc_0p5;
//           doc["nc_1p0"] = spsData.nc_1p0;
//           doc["nc_2p5"] = spsData.nc_2p5;
//           doc["nc_4p0"] = spsData.nc_4p0;
//           doc["nc_10p0"] = spsData.nc_10p0;
//           doc["typical_particle_size"] = spsData.typical_particle_size;

//           if (useSIM)
//             doc["time_unix"] = modem.getCurrentTimestampMs() / 1000; 
//           else
//             doc["time_unix"] = getCurrentTimestampMs() / 1000; });

//   // Add to post queue
//   postQueue.push_back(data);

//   // Ensure queue does not exceed max size
//   if (postQueue.size() > kMaxQueueSize)
//   {
//     postQueue.pop_front();
//     DEBUG_WARN("Post queue full, dropping oldest data");
//   }

//   // Try to send queued data
//   while (!postQueue.empty())
//   {
//     if (trySendQueue(SPS30path))
//     {
//       DEBUG_INFO("Successfully sent queued data");
//       DEBUG_KV("Remaining queue size", postQueue.size());
//     }
//     else
//     {
//       DEBUG_WARN("Failed to send queued data, will retry later");
//       DEBUG_KV("Current queue size", postQueue.size());
//       break;
//     }
//   }
// }

// // Read out the sht41 sensor
// void HandleSHT41Logic()
// {
//   unsigned long now = millis();
//   bool can_measure = now - sht41.last_measurement >= kDataMeasurementInterval;
//   bool can_update = now - sht41.last_update >= kDataTransmissionInterval;

//   if (!can_measure)
//     return;

//   SHT41Data shtData = sht41.readData(10);

//   if (!shtData.available)
//   {
//     DEBUG_WARN("Failed to read from SHT41 sensor");
//     segmentDisplay.setError();
//     return;
//   }

//   segmentDisplay.setTemp(shtData.temperature);
//   segmentDisplay.setHum(shtData.humidity);

//   sht41.last_measurement = now;

//   if (!can_update)
//     return;

//   sht41.last_update = now;

//   // Pretty print for debug
//   String prettyData = JsonBuilder::BuildJson([&](JsonDocument &doc)
//                                              {
//           doc["temperature"] = shtData.temperature;
//           doc["humidity"] = shtData.humidity;
          
//           if (useSIM)
//             doc["time_unix"] = modem.getCurrentTimestampMs() / 1000; 
//           else
//             doc["time_unix"] = getCurrentTimestampMs() / 1000; }, true);

//   DEBUG_BLOCK("Payload");
//   DEBUG_INFO(prettyData);

//   // Compact version for actual POST
//   String data = JsonBuilder::BuildJson([&](JsonDocument &doc)
//                                        {
//           doc["temperature"] = shtData.temperature;
//           doc["humidity"] = shtData.humidity;

//           if (useSIM)
//             doc["time_unix"] = modem.getCurrentTimestampMs() / 1000; 
//           else
//             doc["time_unix"] = getCurrentTimestampMs() / 1000; });

//   // Add to post queue
//   postQueue.push_back(data);

//   // Ensure queue does not exceed max size
//   if (postQueue.size() > kMaxQueueSize)
//   {
//     postQueue.pop_front();
//     DEBUG_WARN("Post queue full, dropping oldest data");
//   }

//   // Try to send queued data
//   while (!postQueue.empty())
//   {
//     if (trySendQueue(SHT41path))
//     {
//       DEBUG_INFO("Successfully sent queued data");
//       DEBUG_KV("Remaining queue size", postQueue.size());
//     }
//     else
//     {
//       DEBUG_WARN("Failed to send queued data, will retry later");
//       DEBUG_KV("Current queue size", postQueue.size());
//       break;
//     }
//   }
// }

// bool trySendQueue(const char *path)
// {
//   if (postQueue.empty())
//     return true;

//   // Always send the oldest entry
//   const String &current = postQueue.front();

//   httpManager.signBody(current.c_str(), credential_manager.GetDeviceKey().c_str());
//   auto [signatureOk, signature] = httpManager.signBody(current.c_str(), credential_manager.GetDeviceKey().c_str());

//   if (!signatureOk)
//   {
//     DEBUG_WARN("Failed to sign request body");
//     return false;
//   }

//   if (useSIM)
//   {
//     // TODO add signature logic to SIM7080

//     if (!modem.ensureConnected())
//     {
//       DEBUG_WARN("Failed to connect to network");
//       strip.stopLoading();
//       strip.startLoading(CRGB::Purple, LEDStrip::_loadingModeType::BLINKING);

//       while (!modem.ensureConnected())
//         ;
//     }

//     if (modem.httpPost(host, path, url, current.c_str()))
//     {
//       postQueue.pop_front();
//       return true;
//     }
//   }
//   else
//   {
//     if (httpManager.post(String(credential_manager.GetDeviceID()),
//                          signature.c_str(),
//                          String(url),
//                          String("/api/data/test"),
//                          current))
//     {
//       postQueue.pop_front();
//       return true;
//     }
//   }

//   return false;
// }

// void initializeTime()
// {
//   DEBUG_SECTION("NTP Sync");
//   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
//   DEBUG_INFO("Waiting for NTP time sync...");
//   struct tm timeinfo;
//   while (!getLocalTime(&timeinfo, 10000))
//   {
//     DEBUG_WARN("Failed to obtain time, retrying...");
//     delay(2000);
//   }
//   DEBUG_OK("Time synchronized");
//   DEBUG_KV("Current time", String(asctime(&timeinfo)));
// }

// uint64_t getCurrentTimestampMs()
// {
//   struct timeval tv;
//   gettimeofday(&tv, nullptr);
//   return static_cast<uint64_t>(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
// }


/**
 * @file      MinimalLowBattPowerOffExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-05-02
 *
 */
#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU  PMU;

bool  pmu_flag = false;
uint32_t loopMillis;

void setFlag(void)
{
    pmu_flag = true;
}

void setup()
{

    Serial.begin(115200);

    // Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    Serial.println();

    /*********************************
     *  step 1 : Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    bool res;
    // Use Wire1
    res = PMU.begin(Wire1, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
    if (!res) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the modem power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }
    

    // I2C sensor call example
    int sda = 13;  // You can also use other IO ports
    int scl = 21;  // You can also use other IO ports
    Wire.begin(sda, scl);

    //**\

    // Other i2c sensors can be externally connected to 13,21

    //**\


    Serial.printf("getID:0x%x\n", PMU.getChipID());

    // Set the minimum common working voltage of the PMU VBUS input,
    // below this value will turn off the PMU
    PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

    // Set the maximum current of the PMU VBUS input,
    // higher than this value will turn off the PMU
    PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);

    // Set VSY off voltage as 2600mV , Adjustment range 2600mV ~ 3300mV
    PMU.setSysPowerDownVoltage(2600);

    // Set the working voltage of the modem, please do not modify the parameters
    PMU.setDC3Voltage(3000);    // SIM7080 Modem main power channel 2700~ 3400V
    PMU.enableDC3();

    // Modem GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      // The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();



    /*********************************
     * step 2 : Enable internal ADC detection
    ***********************************/
    PMU.enableBattDetection();
    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.enableSystemVoltageMeasure();


    /*********************************
     * step 3 : Set PMU interrupt
    ***********************************/
    pinMode(PMU_INPUT_PIN, INPUT);
    attachInterrupt(PMU_INPUT_PIN, setFlag, FALLING);

    // Disable all interrupts
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    // Clear all interrupt flags
    PMU.clearIrqStatus();
    // Enable the required interrupt function
    PMU.enableIRQ(
        XPOWERS_AXP2101_BAT_INSERT_IRQ     | XPOWERS_AXP2101_BAT_REMOVE_IRQ     |   // BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ    | XPOWERS_AXP2101_VBUS_REMOVE_IRQ    |   // VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ     | XPOWERS_AXP2101_PKEY_LONG_IRQ      |   // POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ   | XPOWERS_AXP2101_BAT_CHG_START_IRQ  |   // CHARGE
        XPOWERS_AXP2101_WARNING_LEVEL1_IRQ | XPOWERS_AXP2101_WARNING_LEVEL2_IRQ     // LOW BATTERY
    );

    /*********************************
     * step 4 : Set PMU Charger params
    ***********************************/
    // Set the precharge charging current
    PMU.setPrechargeCurr(XPOWERS_AXP2101_PRECHARGE_50MA);
    // Set constant current charge current limit
    PMU.setChargerConstantCurr(XPOWERS_AXP2101_CHG_CUR_200MA);
    // Set stop charging termination current
    PMU.setChargerTerminationCurr(XPOWERS_AXP2101_CHG_ITERM_25MA);

    // Set charge cut-off voltage
    PMU.setChargeTargetVoltage(XPOWERS_AXP2101_CHG_VOL_4V1);


    /*
    The default setting is CHGLED is automatically controlled by the PMU.
    - XPOWERS_CHG_LED_OFF,
    - XPOWERS_CHG_LED_BLINK_1HZ,
    - XPOWERS_CHG_LED_BLINK_4HZ,
    - XPOWERS_CHG_LED_ON,
    - XPOWERS_CHG_LED_CTRL_CHG,
    * */
    PMU.setChargingLedMode(XPOWERS_CHG_LED_CTRL_CHG);



    /*********************************
     * step 5 : Set PMU low battery params
    ***********************************/

    // Get the default low pressure warning percentage setting
    uint8_t low_warn_per = PMU.getLowBatWarnThreshold();
    Serial.printf("Default low battery warning threshold is %d percentage\n", low_warn_per);

    //
    // setLowBatWarnThreshold Range:  5% ~ 20%
    // The following data is obtained from actual testing , Please see the description below for the test method.
    // 20% ~= 3.7v
    // 15% ~= 3.6v
    // 10% ~= 3.55V
    // 5%  ~= 3.5V
    // 1%  ~= 3.4V
    PMU.setLowBatWarnThreshold(5); // Set to trigger interrupt when reaching 5%

    // Get the low voltage warning percentage setting
    low_warn_per = PMU.getLowBatWarnThreshold();
    Serial.printf("Set low battery warning threshold is %d percentage\n", low_warn_per);

    // Get the default low voltage shutdown percentage setting
    uint8_t low_shutdown_per = PMU.getLowBatShutdownThreshold();
    Serial.printf("Default low battery shutdown threshold is %d percentage\n", low_shutdown_per);

    // setLowBatShutdownThreshold Range:  0% ~ 15%
    // The following data is obtained from actual testing , Please see the description below for the test method.
    // 15% ~= 3.6v
    // 10% ~= 3.55V
    // 5%  ~= 3.5V
    // 1%  ~= 3.4V
    PMU.setLowBatShutdownThreshold(1);  // Set to trigger interrupt when reaching 1%

    // Get the low voltage shutdown percentage setting
    low_shutdown_per = PMU.getLowBatShutdownThreshold();
    Serial.printf("Set low battery shutdown threshold is %d percentage\n", low_shutdown_per);



    /*
    *
    *    Measurement methods:
    *    1. Connect the battery terminal to a voltage stabilizing source
    *    2. Set voltage test voltage
    *    3. Press PWR to boot
    *    4. Read the serial output voltage percentage
    *
    *   If a voltage regulator is connected during testing and the voltage is slowly reduced,
    *   the voltage percentage will not change immediately. It will take a while to slowly decrease.
    *   In actual production, it needs to be adjusted according to the actual situation.
    * * * */
}

void loop()
{
    if (pmu_flag) {

        pmu_flag = false;

        // Get PMU Interrupt Status Register
        uint32_t status = PMU.getIrqStatus();

        if (PMU.isVbusInsertIrq()) {
            Serial.println("isVbusInsert");
        }
        if (PMU.isVbusRemoveIrq()) {
            Serial.println("isVbusRemove");
        }
        if (PMU.isBatInsertIrq()) {
            Serial.println("isBatInsert");
        }
        if (PMU.isBatRemoveIrq()) {
            Serial.println("isBatRemove");
        }
        if (PMU.isPekeyShortPressIrq()) {
            Serial.println("isPekeyShortPress");
        }
        if (PMU.isPekeyLongPressIrq()) {
            Serial.println("isPekeyLongPress");
        }

        // When the set low-voltage battery percentage warning threshold is reached,
        // set the threshold through getLowBatWarnThreshold( 5% ~ 20% )
        if (PMU.isDropWarningLevel2Irq()) {
            Serial.println("The voltage percentage has reached the low voltage warning threshold!!!");
        }

        // When the set low-voltage battery percentage shutdown threshold is reached
        // set the threshold through setLowBatShutdownThreshold()
        if (PMU.isDropWarningLevel1Irq()) {
            int i = 4;
            while (i--) {
                Serial.printf("The voltage percentage has reached the low voltage shutdown threshold and will shut down in %d seconds.\n", i);
            }
            // Turn off all power supplies, leaving only the RTC power supply. The RTC power supply cannot be turned off.
            PMU.shutdown();
        }


        // For more interrupt sources, please check XPowersLib


        // Clear PMU Interrupt Status Register
        PMU.clearIrqStatus();

    }


    if (millis() - loopMillis > 3000) {
        Serial.print("getBattVoltage:"); Serial.print(PMU.getBattVoltage()); Serial.println("mV");
        Serial.print("getVbusVoltage:"); Serial.print(PMU.getVbusVoltage()); Serial.println("mV");
        Serial.print("getSystemVoltage:"); Serial.print(PMU.getSystemVoltage()); Serial.println("mV");

        // The battery percentage may be inaccurate at first use, the PMU will automatically
        // learn the battery curve and will automatically calibrate the battery percentage
        // after a charge and discharge cycle
        if (PMU.isBatteryConnect()) {
            Serial.print("getBatteryPercent:"); Serial.print(PMU.getBatteryPercent()); Serial.println("%");
        }
        Serial.println();
        loopMillis = millis();
    }
}