#include <Arduino.h>

#ifdef TESTSIM_SERIAL_ONLY
void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("testsim serial heartbeat");
  delay(1000);
}
#elif defined(TESTSIM_PMU_LED_ONLY)
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU PMU;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
}

void loop() {
  PMU.setChargingLedMode(ledOn ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
  Serial.println(ledOn ? "pmu led on" : "pmu led off");
  ledOn = !ledOn;
  delay(1000);
}
#elif defined(TESTSIM_VBUS_WATCH)
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU PMU;
bool ledOn = false;

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println(F("testsim VBUS insertion watcher"));
  Serial.println(F("No DC3, no modem PWRKEY, no SIM stack."));

  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println(F("PMU begin failed."));
    while (true) {
      delay(1000);
    }
  }

  PMU.disableCellbatteryCharge();
  PMU.enableBattDetection();
  PMU.enableBattVoltageMeasure();
  PMU.enableVbusVoltageMeasure();
  PMU.enableSystemVoltageMeasure();
  PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

  Serial.println(F("Battery charging disabled for Otii/bench-source testing."));
  Serial.println(F("Plug or unplug USB-C while this runs; CHG LED toggles while alive."));
}

void loop() {
  PMU.setChargingLedMode(ledOn ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
  ledOn = !ledOn;

  Serial.print(F("alive, vbusIn="));
  Serial.print(PMU.isVbusIn() ? F("yes") : F("no"));
  Serial.print(F(", vbusGood="));
  Serial.print(PMU.isVbusGood() ? F("yes") : F("no"));
  Serial.print(F(", vbus="));
  Serial.print(PMU.getVbusVoltage());
  Serial.print(F("mV, battConnected="));
  Serial.print(PMU.isBatteryConnect() ? F("yes") : F("no"));
  Serial.print(F(", batt="));
  Serial.print(PMU.getBattVoltage());
  Serial.print(F("mV, sys="));
  Serial.print(PMU.getSystemVoltage());
  Serial.print(F("mV, charging="));
  Serial.print(PMU.isCharging() ? F("yes") : F("no"));
  Serial.print(F(", discharging="));
  Serial.println(PMU.isDischarge() ? F("yes") : F("no"));

  delay(1000);
}
#elif defined(TESTSIM_DC3_ONLY)
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

#ifndef TESTSIM_DC3_MV
#define TESTSIM_DC3_MV 3000
#endif

XPowersPMU PMU;
bool ledOn = false;

void ledBlink(uint8_t count, uint16_t onMs = 160, uint16_t offMs = 160) {
  for (uint8_t i = 0; i < count; i++) {
    PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);
    delay(onMs);
    PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);
    delay(offMs);
  }
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println();
  Serial.println(F("testsim DC3 rail diagnostic"));
  Serial.println(F("No modem PWRKEY, no SIM stack, no BLDO2."));
#ifdef TESTSIM_ENABLE_BLDO1
  Serial.println(F("BLDO1 level shifter rail will be enabled before DC3."));
#else
  Serial.println(F("BLDO1 level shifter rail stays off."));
#endif
#ifdef TESTSIM_MODEM_PINS_SAFE
  Serial.println(F("Modem PWRKEY and DTR will be held LOW before DC3."));
#endif
#ifdef TESTSIM_DC3_PWM
  Serial.println(F("DC3 will be forced to PWM mode before enable."));
#endif
#ifdef TESTSIM_DC3_NO_UVP_SHUTDOWN
  Serial.println(F("DC3 low-voltage PMIC turn-off will be disabled before enable."));
#endif
  Serial.print(F("Target DC3 voltage: "));
  Serial.print(TESTSIM_DC3_MV);
  Serial.println(F(" mV"));

  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println(F("PMU begin failed."));
    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("Disable battery charging for Otii/bench-source testing."));
  PMU.disableCellbatteryCharge();

  Serial.println(F("PMU alive. CHG LED blinks at 1 Hz before DC3 is enabled."));
  PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);
  delay(8000);

  Serial.println(F("Set USB input limits."));
  PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);
  PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

  Serial.println(F("Make sure DC3 is off first."));
  PMU.disableDC3();
  delay(500);

#ifdef TESTSIM_ENABLE_BLDO1
  Serial.println(F("Enable BLDO1 level shifter at 3300 mV."));
  Serial.flush();
  PMU.setBLDO1Voltage(3300);
  PMU.enableBLDO1();
  Serial.println(F("BLDO1 enabled OK."));
  Serial.flush();
  delay(1000);
#endif

#ifdef TESTSIM_MODEM_PINS_SAFE
  Serial.println(F("Set modem control pins safe before DC3: PWRKEY=LOW, DTR=LOW."));
  pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_RI_PIN, INPUT);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
  Serial.flush();
  delay(1000);
#endif

#ifdef TESTSIM_DC3_PWM
  Serial.println(F("Force DC3 PWM mode."));
  PMU.settDC3WorkModeToPwm(true);
  Serial.flush();
  delay(500);
#endif

#ifdef TESTSIM_DC3_NO_UVP_SHUTDOWN
  Serial.println(F("Disable DC3 low-voltage PMIC turn-off."));
  PMU.disableDC3LowVoltageTurnOff();
  PMU.setDC3LowVoltagePowerDowm(false);
  Serial.flush();
  delay(500);
#endif

  Serial.println(F("Set DC3 voltage."));
  PMU.setDC3Voltage(TESTSIM_DC3_MV);
  Serial.print(F("PMU reports DC3 voltage: "));
  Serial.print(PMU.getDC3Voltage());
  Serial.println(F(" mV"));

  Serial.println(F("Three fast CHG LED blinks, then DC3 enable."));
  ledBlink(3);
  delay(1500);

#ifdef TESTSIM_NO_PMU_READBACK
  Serial.println(F("Set CHG LED to 1 Hz blink before DC3. No PMU I2C after DC3."));
  PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);
  Serial.flush();
  delay(1000);
#endif

  Serial.println(F("Enabling DC3 now."));
  Serial.flush();
  PMU.enableDC3();
  Serial.println(F("DC3 enable returned. If the board stays alive, CHG LED now toggles."));
  Serial.flush();
}

void loop() {
#ifdef TESTSIM_NO_PMU_READBACK
#ifdef TESTSIM_DC3_NO_UVP_SHUTDOWN
  Serial.println(F("bldo1dc3nouvp alive, no PMU readback after DC3"));
#elif defined(TESTSIM_DC3_PWM)
  Serial.println(F("bldo1dc3pwm alive, no PMU readback after DC3"));
#elif defined(TESTSIM_MODEM_PINS_SAFE)
  Serial.println(F("bldo1dc3pins alive, no PMU readback after DC3"));
#else
  Serial.println(F("bldo1dc3hold alive, no PMU readback after DC3"));
#endif
  delay(1000);
#else
  PMU.setChargingLedMode(ledOn ? XPOWERS_CHG_LED_ON : XPOWERS_CHG_LED_OFF);
  Serial.print(F("dc3test alive, DC3 enabled="));
  Serial.print(PMU.isEnableDC3() ? F("yes") : F("no"));
  Serial.print(F(", voltage="));
  Serial.print(PMU.getDC3Voltage());
  Serial.println(F(" mV"));
  ledOn = !ledOn;
  delay(1000);
#endif
}
#elif defined(TESTSIM_MODEM_SCAN)
#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

#ifndef TESTSIM_DC3_MV
#define TESTSIM_DC3_MV 3000
#endif

XPowersPMU PMU;
uint32_t bridgeBaud = 115200;

int modemRxPin() {
#ifdef TESTSIM_MODEM_UART_SWAP
  return BOARD_MODEM_TXD_PIN;
#else
  return BOARD_MODEM_RXD_PIN;
#endif
}

int modemTxPin() {
#ifdef TESTSIM_MODEM_UART_SWAP
  return BOARD_MODEM_RXD_PIN;
#else
  return BOARD_MODEM_TXD_PIN;
#endif
}

String readSerial1Response(uint32_t timeoutMs) {
  String response;
  const uint32_t startedAt = millis();

  while (millis() - startedAt < timeoutMs) {
    while (Serial1.available()) {
      const char c = static_cast<char>(Serial1.read());
      Serial.write(c);
      response += c;
    }
    delay(5);
  }

  return response;
}

void clearSerial1() {
  while (Serial1.available()) {
    Serial1.read();
  }
}

bool probeAt(uint32_t baud) {
  Serial1.end();
  delay(80);
  Serial1.begin(baud, SERIAL_8N1, modemRxPin(), modemTxPin());
  delay(120);
  clearSerial1();

  for (uint8_t attempt = 1; attempt <= 3; attempt++) {
    Serial.print(F("Probe AT at "));
    Serial.print(baud);
    Serial.print(F(" baud, attempt "));
    Serial.println(attempt);
    Serial.flush();

    Serial1.print("AT\r\n");
    const String response = readSerial1Response(1400);
    if (response.length() == 0) {
      Serial.println(F("  no bytes"));
    }
    if (response.indexOf("OK") >= 0) {
      Serial.print(F("  AT OK at "));
      Serial.print(baud);
      Serial.println(F(" baud"));
      bridgeBaud = baud;
      return true;
    }
    delay(250);
  }

  return false;
}

bool scanBauds(const __FlashStringHelper *label) {
  static const uint32_t bauds[] = {115200, 9600, 19200, 38400,
                                   57600,  230400, 460800};

  Serial.println();
  Serial.println(label);
  for (uint8_t i = 0; i < sizeof(bauds) / sizeof(bauds[0]); i++) {
    if (probeAt(bauds[i])) {
      return true;
    }
  }
  return false;
}

bool scanAfterDelay(const __FlashStringHelper *label, uint32_t delayMs = 8000) {
  Serial.print(F("Wait "));
  Serial.print(delayMs / 1000);
  Serial.println(F("s for modem boot."));
  delay(delayMs);
  return scanBauds(label);
}

void lilygoPowerKeyPulse() {
  Serial.println(F("Pulse PWRKEY using LilyGO sequence: LOW 100ms, HIGH 1200ms, LOW."));
  Serial.flush();
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  delay(100);
  digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
  delay(1200);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
}

void activeHighPowerKeyPulse(uint16_t highMs) {
  Serial.print(F("Pulse PWRKEY active-HIGH for "));
  Serial.print(highMs);
  Serial.println(F("ms: LOW idle, HIGH pulse, LOW idle."));
  Serial.flush();
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  delay(250);
  digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
  delay(highMs);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
}

void activeLowPowerKeyPulse(uint16_t lowMs) {
  Serial.print(F("Pulse PWRKEY active-LOW for "));
  Serial.print(lowMs);
  Serial.println(F("ms: HIGH idle, LOW pulse, HIGH idle."));
  Serial.flush();
  digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
  delay(250);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  delay(lowMs);
  digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
}

bool setupModemPower() {
  Serial.println();
  Serial.println(F("testsim SIM7080G wake/baud scan"));
#ifdef TESTSIM_MODEM_UART_SWAP
  Serial.println(F("UART pins are swapped for this diagnostic."));
#endif
#ifdef TESTSIM_ENABLE_BLDO2
  Serial.println(F("BLDO2 GNSS antenna rail will be enabled for this diagnostic."));
#endif
  Serial.print(F("Target DC3 voltage: "));
  Serial.print(TESTSIM_DC3_MV);
  Serial.println(F(" mV"));

  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println(F("PMU begin failed."));
    return false;
  }

  PMU.disableCellbatteryCharge();
  PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);
  PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
  PMU.setChargingLedMode(XPOWERS_CHG_LED_BLINK_1HZ);

  pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_RI_PIN, INPUT);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

  PMU.disableDC3();
  delay(200);

  PMU.setBLDO1Voltage(3300);
  PMU.enableBLDO1();

#ifdef TESTSIM_ENABLE_BLDO2
  PMU.setBLDO2Voltage(3300);
  PMU.enableBLDO2();
#endif

  PMU.disableDC3LowVoltageTurnOff();
  PMU.setDC3LowVoltagePowerDowm(false);

  PMU.setDC3Voltage(TESTSIM_DC3_MV);
  Serial.print(F("PMU reports DC3 voltage: "));
  Serial.print(PMU.getDC3Voltage());
  Serial.println(F(" mV"));

  PMU.enableDC3();
  PMU.disableTSPinMeasure();

  Serial.println(F("DC3 on, BLDO1 on. Waiting before UART probe."));
  Serial.flush();
  delay(5000);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  if (!setupModemPower()) {
    while (true) {
      delay(1000);
    }
  }

  bool found = scanBauds(F("Passive baud scan before PWRKEY pulse."));

#ifdef TESTSIM_MODEM_KEYSCAN
  if (!found) {
    lilygoPowerKeyPulse();
    found = scanAfterDelay(F("Baud scan after LilyGO PWRKEY pulse."));
  }

  if (!found) {
    activeHighPowerKeyPulse(2500);
    found = scanAfterDelay(F("Baud scan after active-HIGH 2500ms pulse."));
  }

  if (!found) {
    activeHighPowerKeyPulse(5000);
    found = scanAfterDelay(F("Baud scan after active-HIGH 5000ms pulse."), 12000);
  }

  if (!found) {
    activeLowPowerKeyPulse(1500);
    found = scanAfterDelay(F("Baud scan after active-LOW 1500ms pulse."));
  }

  if (!found) {
    activeLowPowerKeyPulse(3000);
    found = scanAfterDelay(F("Baud scan after active-LOW 3000ms pulse."), 12000);
  }
#else
  if (!found) {
    lilygoPowerKeyPulse();
    delay(8000);
    found = scanBauds(F("Baud scan after PWRKEY pulse."));
  }
#endif

  if (!found) {
    Serial.println(F("Set DTR HIGH and scan once more."));
    digitalWrite(BOARD_MODEM_DTR_PIN, HIGH);
    delay(1000);
    found = scanBauds(F("Baud scan with DTR HIGH."));
  }

  if (!found) {
    Serial.println(F("No AT response found on scanned baud rates."));
    bridgeBaud = 115200;
  }

  Serial.print(F("AT bridge ready at "));
  Serial.print(bridgeBaud);
  Serial.println(F(" baud. Use Both NL & CR."));
  Serial1.end();
  delay(80);
  Serial1.begin(bridgeBaud, SERIAL_8N1, modemRxPin(), modemTxPin());
}

void loop() {
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }

  while (Serial.available()) {
    Serial1.write(Serial.read());
  }

  static uint32_t lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 5000) {
    Serial.println(F("modemscan bridge alive"));
    lastHeartbeat = millis();
  }

  delay(1);
}
#else

#include <Wire.h>

#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

#ifndef TINY_GSM_MODEM_SIM7080
#define TINY_GSM_MODEM_SIM7080
#endif

#ifndef TINY_GSM_RX_BUFFER
#define TINY_GSM_RX_BUFFER 1024
#endif

#define SerialAT Serial1
#include <TinyGsmClient.h>

#ifndef TESTSIM_APN
#define TESTSIM_APN "iot.1nce.net"
#endif

#ifndef TESTSIM_GPRS_USER
#define TESTSIM_GPRS_USER ""
#endif

#ifndef TESTSIM_GPRS_PASS
#define TESTSIM_GPRS_PASS ""
#endif

#ifndef TESTSIM_PREFERRED_MODE
#define TESTSIM_PREFERRED_MODE 1
#endif

#ifndef TESTSIM_NETWORK_MODE
#define TESTSIM_NETWORK_MODE 2
#endif

#ifndef TESTSIM_RAT_NAME
#define TESTSIM_RAT_NAME "CAT-M"
#endif

#ifndef TESTSIM_HTTP_HOST
#define TESTSIM_HTTP_HOST "vsh.pp.ua"
#endif

#ifndef TESTSIM_HTTP_RESOURCE
#define TESTSIM_HTTP_RESOURCE "/TinyGSM/logo.txt"
#endif

#ifndef TESTSIM_PING_HOST
#define TESTSIM_PING_HOST "8.8.8.8"
#endif

#ifndef TESTSIM_REGISTRATION_TIMEOUT_MS
#define TESTSIM_REGISTRATION_TIMEOUT_MS 180000UL
#endif

#ifndef TESTSIM_PRE_PMU_DELAY_MS
#define TESTSIM_PRE_PMU_DELAY_MS 30000UL
#endif

namespace {

constexpr uint8_t kModeCatM = 1;
constexpr uint8_t kModeNbIot = 2;
constexpr uint8_t kModeCatMAndNbIot = 3;

XPowersPMU PMU;
TinyGsm modem(SerialAT);

const char *const kApn = TESTSIM_APN;
const char *const kGprsUser = TESTSIM_GPRS_USER;
const char *const kGprsPass = TESTSIM_GPRS_PASS;
const char *const kHttpHost = TESTSIM_HTTP_HOST;
const char *const kHttpResource = TESTSIM_HTTP_RESOURCE;
const char *const kPingHost = TESTSIM_PING_HOST;

void printStep(const __FlashStringHelper *title) {
  Serial.println();
  Serial.println(F("============================================================"));
  Serial.println(title);
  Serial.println(F("============================================================"));
}

void drainModemInput() {
  while (SerialAT.available()) {
    SerialAT.read();
  }
}

String readModemResponse(uint32_t idleTimeoutMs, uint32_t totalTimeoutMs) {
  String response;
  response.reserve(512);

  const uint32_t startedAt = millis();
  uint32_t lastByteAt = millis();

  while ((millis() - startedAt < totalTimeoutMs) &&
         (millis() - lastByteAt < idleTimeoutMs)) {
    while (SerialAT.available()) {
      char c = static_cast<char>(SerialAT.read());
      Serial.write(c);
      if (response.length() < 2048) {
        response += c;
      }
      lastByteAt = millis();
    }
    delay(1);
  }

  if (!response.endsWith("\n")) {
    Serial.println();
  }

  return response;
}

String sendRawAT(const char *command, uint32_t totalTimeoutMs = 3000,
                 uint32_t idleTimeoutMs = 700) {
  drainModemInput();
  Serial.print(F(">> "));
  Serial.println(command);
  SerialAT.print(command);
  SerialAT.print("\r\n");
  return readModemResponse(idleTimeoutMs, totalTimeoutMs);
}

const char *registrationStatusName(SIM70xxRegStatus status) {
  switch (status) {
  case REG_NO_RESULT:
    return "no result";
  case REG_UNREGISTERED:
    return "not registered";
  case REG_SEARCHING:
    return "searching";
  case REG_DENIED:
    return "denied";
  case REG_OK_HOME:
    return "registered home";
  case REG_OK_ROAMING:
    return "registered roaming";
  case REG_UNKNOWN:
    return "unknown";
  default:
    return "unexpected";
  }
}

bool isRegistered(SIM70xxRegStatus status) {
  return status == REG_OK_HOME || status == REG_OK_ROAMING;
}

void printBanner() {
  Serial.println();
  Serial.println(F("LilyGO T-SIM7080G SIM bring-up"));
  Serial.print(F("APN: "));
  Serial.println(kApn);
  Serial.print(F("Preferred radio access: "));
  Serial.println(F(TESTSIM_RAT_NAME));
  Serial.println(F("Serial monitor: 115200 baud, Both NL & CR for AT bridge"));
  Serial.println(F("Insert the SIM before power-up or reset the board after inserting it."));
}

void waitForSerialMonitor() {
  delay(1000);
  Serial.println();
  Serial.println(F("Firmware startup pause before PMU setup."));
  Serial.println(F("If the board powers off after this point, note the last printed step."));

  const uint32_t seconds = TESTSIM_PRE_PMU_DELAY_MS / 1000UL;
  for (uint32_t remaining = seconds; remaining > 0; remaining--) {
    Serial.print(F("Starting PMU/modem setup in "));
    Serial.print(remaining);
    Serial.println(F("s..."));
    delay(1000);
  }
}

bool initializePower() {
  printStep(F("1. AXP2101 power setup"));
  Serial.flush();

  Serial.println(F("PMU: begin Wire on SDA=15 SCL=7"));
  Serial.flush();
  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println(F("Failed to initialize AXP2101 PMU."));
    Serial.flush();
    return false;
  }
  Serial.println(F("PMU: begin OK"));
  Serial.flush();

  Serial.println(F("PMU: set USB input current limit to 1500 mA"));
  Serial.flush();
  PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);
  Serial.println(F("PMU: USB input current limit OK"));
  Serial.flush();

  Serial.println(F("PMU: set USB input voltage limit to 4.36 V"));
  Serial.flush();
  PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
  Serial.println(F("PMU: USB input voltage limit OK"));
  Serial.flush();

  Serial.println(F("PMU: hold modem control pins safe before rails"));
  Serial.flush();
  pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_RI_PIN, INPUT);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println(F("PMU: cold boot, cycling DC3 modem rail"));
    Serial.flush();
    PMU.disableDC3();
    delay(200);
  }

  // BLDO1 powers the UART level shifter between ESP32-S3 and SIM7080G.
  Serial.println(F("PMU: enable BLDO1 level shifter at 3300 mV"));
  Serial.flush();
  PMU.setBLDO1Voltage(3300);
  PMU.enableBLDO1();

  Serial.println(F("PMU: disable DC3 low-voltage PMIC turn-off"));
  Serial.flush();
  PMU.disableDC3LowVoltageTurnOff();
  PMU.setDC3LowVoltagePowerDowm(false);

  // DC3 is the SIM7080G modem rail on the LilyGO board.
  Serial.println(F("PMU: set DC3 modem rail voltage to 3000 mV"));
  Serial.flush();
  PMU.setDC3Voltage(3000);
  Serial.println(F("PMU: DC3 voltage set OK"));
  Serial.flush();
  delay(3000);

  Serial.println(F("PMU: enable DC3 modem rail"));
  Serial.flush();
  PMU.enableDC3();
  Serial.println(F("PMU: DC3 enabled OK"));
  Serial.flush();
  for (uint8_t second = 1; second <= 10; second++) {
    Serial.print(F("PMU: DC3 hold test "));
    Serial.print(second);
    Serial.println(F("/10"));
    Serial.flush();
    delay(1000);
  }

  // BLDO2 powers the GNSS antenna rail. It is not needed for cellular data,
  // and this board powers off/hangs when this rail is touched.
  Serial.println(F("PMU: skip BLDO2 GNSS antenna rail"));
  Serial.flush();

  Serial.println(F("PMU: disable TS pin measure"));
  Serial.flush();
  PMU.disableTSPinMeasure();

  Serial.println(F("PMU ready: DC3 modem rail on, BLDO1 level shifter on."));
  Serial.flush();
  return true;
}

void pulsePowerKey() {
  // LilyGO's examples use this sequence for the board-level PWRKEY circuit.
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  delay(100);
  digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
  delay(1200);
  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  delay(3000);
}

bool startModem() {
  printStep(F("2. SIM7080G UART and power key"));

  SerialAT.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN,
                 BOARD_MODEM_TXD_PIN);

  pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_RI_PIN, INPUT);

  digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
  digitalWrite(BOARD_MODEM_DTR_PIN, LOW);

  for (uint8_t cycle = 0; cycle < 5; cycle++) {
    Serial.print(F("Waiting for AT response"));
    for (uint8_t attempt = 0; attempt < 8; attempt++) {
      if (modem.testAT(1000)) {
        Serial.println();
        Serial.println(F("Modem responded to AT."));
        sendRawAT("ATE0", 2000);
        sendRawAT("AT+CMEE=2", 2000);
        sendRawAT("ATI", 3000);
        return true;
      }
      Serial.print('.');
    }

    Serial.println();
    Serial.println(F("No AT response yet, pulsing PWRKEY."));
    pulsePowerKey();
  }

  Serial.println(F("Modem did not respond after multiple PWRKEY attempts."));
  return false;
}

bool checkSim() {
  printStep(F("3. SIM card checks"));

  sendRawAT("AT+CPIN?", 3000);
  SimStatus status = modem.getSimStatus();
  if (status != SIM_READY) {
    Serial.print(F("SIM status is not ready: "));
    Serial.println(static_cast<int>(status));
    Serial.println(F("Power off, insert a Cat-M/NB-IoT SIM, then boot again."));
    return false;
  }

  Serial.println(F("SIM is ready."));
  Serial.print(F("CCID: "));
  Serial.println(modem.getSimCCID());
  Serial.print(F("IMSI: "));
  Serial.println(modem.getIMSI());
  Serial.print(F("IMEI: "));
  Serial.println(modem.getIMEI());
  return true;
}

bool configureRatAndApn() {
  printStep(F("4. RAT and APN configuration"));

  Serial.println(F("Turning RF off before changing modem mode."));
  modem.sendAT("+CFUN=0");
  if (modem.waitResponse(20000UL) != 1) {
    Serial.println(F("Warning: CFUN=0 did not return OK."));
  }

  if (!modem.setNetworkMode(TESTSIM_NETWORK_MODE)) {
    Serial.println(F("Failed to set network mode (CNMP)."));
    return false;
  }

  if (!modem.setPreferredMode(TESTSIM_PREFERRED_MODE)) {
    Serial.println(F("Failed to set preferred SIM7080 radio mode (CMNB)."));
    return false;
  }

  Serial.print(F("Network mode now: "));
  Serial.println(modem.getNetworkMode());
  Serial.print(F("Preferred mode now: "));
  Serial.println(modem.getPreferredMode());

  modem.sendAT("+CGDCONT=1,\"IP\",\"", kApn, "\"");
  if (modem.waitResponse() != 1) {
    Serial.println(F("Failed to set PDP APN with CGDCONT."));
    return false;
  }

  if (strlen(kGprsUser) > 0 || strlen(kGprsPass) > 0) {
    modem.sendAT("+CNCFG=0,1,\"", kApn, "\",\"", kGprsUser, "\",\"",
                 kGprsPass, "\",3");
  } else {
    modem.sendAT("+CNCFG=0,1,\"", kApn, "\"");
  }

  if (modem.waitResponse() != 1) {
    Serial.println(F("Failed to configure application APN with CNCFG."));
    return false;
  }

  Serial.println(F("Turning RF back on."));
  modem.sendAT("+CFUN=1");
  if (modem.waitResponse(20000UL) != 1) {
    Serial.println(F("Failed to enable RF with CFUN=1."));
    return false;
  }

  sendRawAT("AT+CGDCONT?", 3000);
  sendRawAT("AT+CNCFG?", 3000);
  return true;
}

bool waitForRegistration() {
  printStep(F("5. LTE registration"));

  uint32_t startedAt = millis();
  uint32_t lastDiagnosticAt = 0;

  while (millis() - startedAt < TESTSIM_REGISTRATION_TIMEOUT_MS) {
    SIM70xxRegStatus status = modem.getRegistrationStatus();
    int16_t signalQuality = modem.getSignalQuality();

    Serial.print(F("Registration: "));
    Serial.print(registrationStatusName(status));
    Serial.print(F(" ("));
    Serial.print(static_cast<int>(status));
    Serial.print(F("), CSQ: "));
    Serial.println(signalQuality);

    if (isRegistered(status)) {
      Serial.println(F("Registered on LTE network."));
      Serial.print(F("Operator: "));
      Serial.println(modem.getOperator());
      return true;
    }

    if (millis() - lastDiagnosticAt > 15000UL) {
      sendRawAT("AT+CEREG?", 3000);
      sendRawAT("AT+CGREG?", 3000);
      sendRawAT("AT+COPS?", 3000);
      sendRawAT("AT+CSQ", 3000);
      lastDiagnosticAt = millis();
    }

    delay(3000);
  }

  Serial.println(F("Registration timed out."));
  Serial.println(F("Try the other PlatformIO environment: catm, nbiot, or auto."));
  return false;
}

bool activateBearer() {
  printStep(F("6. PDP bearer activation"));

  sendRawAT("AT+CNACT?", 5000);
  if (modem.isGprsConnected()) {
    Serial.println(F("Bearer was already active."));
    Serial.print(F("Local IP: "));
    Serial.println(modem.localIP());
    sendRawAT("AT+CNACT?", 5000);
    return true;
  }

  Serial.println(F("Activating bearer with TinyGSM SIM7080 flow."));
  if (!modem.gprsConnect(kApn, kGprsUser, kGprsPass)) {
    Serial.println(F("Bearer activation failed."));
    sendRawAT("AT+CGATT?", 5000);
    sendRawAT("AT+CNACT?", 5000);
    return false;
  }

  Serial.println(F("Bearer is active."));
  Serial.print(F("Local IP: "));
  Serial.println(modem.localIP());
  sendRawAT("AT+CNACT?", 5000);
  return true;
}

bool syncNetworkTime() {
  printStep(F("7. Network time"));

  modem.sendAT("+CLTS=1");
  if (modem.waitResponse(10000UL) != 1) {
    Serial.println(F("CLTS command failed."));
    return false;
  }

  sendRawAT("AT+CCLK?", 5000);
  return true;
}

bool runPingTest() {
  printStep(F("8. Ping test"));

  String command = "AT+SNPING4=\"";
  command += kPingHost;
  command += "\",1,16,5000";
  String response = sendRawAT(command.c_str(), 12000, 1500);
  return response.indexOf("+SNPING4:") >= 0 && response.indexOf("ERROR") < 0;
}

bool runHttpTest() {
  printStep(F("9. Plain HTTP GET test"));

  TinyGsmClient client(modem, 0);
  Serial.print(F("Connecting to http://"));
  Serial.print(kHttpHost);
  Serial.println(kHttpResource);

  if (!client.connect(kHttpHost, 80, 30)) {
    Serial.println(F("HTTP TCP connection failed."));
    return false;
  }

  client.print(String("GET ") + kHttpResource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + kHttpHost + "\r\n");
  client.print("Connection: close\r\n\r\n");

  uint32_t startedAt = millis();
  while (client.connected() && !client.available() &&
         millis() - startedAt < 30000UL) {
    delay(100);
  }

  size_t bytesRead = 0;
  uint32_t lastByteAt = millis();
  while ((client.connected() || client.available()) &&
         millis() - lastByteAt < 10000UL) {
    while (client.available()) {
      char c = static_cast<char>(client.read());
      if (bytesRead < 1200) {
        Serial.write(c);
      }
      bytesRead++;
      lastByteAt = millis();
    }
    delay(1);
  }

  client.stop();
  Serial.println();
  Serial.print(F("HTTP bytes received: "));
  Serial.println(bytesRead);
  return bytesRead > 0;
}

void enterAtBridge() {
  printStep(F("AT bridge ready"));
  Serial.println(F("Type AT commands in the serial monitor using Both NL & CR."));
  Serial.println(F("Useful checks: AT+CEREG?, AT+CNACT?, AT+CSQ, AT+COPS?"));

  while (true) {
    while (SerialAT.available()) {
      Serial.write(SerialAT.read());
    }
    while (Serial.available()) {
      SerialAT.write(Serial.read());
    }
    delay(1);
  }
}

} // namespace

void setup() {
  Serial.begin(115200);

  waitForSerialMonitor();
  printBanner();

  bool ok = initializePower() && startModem() && checkSim() &&
            configureRatAndApn() && waitForRegistration() &&
            activateBearer() && syncNetworkTime();

  if (ok) {
    bool pingOk = runPingTest();
    Serial.print(F("Ping test: "));
    Serial.println(pingOk ? F("PASS") : F("CHECK LOG"));

    bool httpOk = runHttpTest();
    Serial.print(F("HTTP test: "));
    Serial.println(httpOk ? F("PASS") : F("CHECK LOG"));

    if (pingOk && httpOk) {
      printStep(F("SIM setup is working"));
      Serial.println(F("The modem registered, got an IP, pinged, and fetched HTTP."));
    } else {
      printStep(F("SIM setup partially works"));
      Serial.println(F("Registration and bearer worked, but a data-path test needs checking."));
    }
  } else {
    printStep(F("SIM setup failed"));
    Serial.println(F("Use the diagnostics above, then try the AT bridge below."));
  }

  enterAtBridge();
}

void loop() {}
#endif
