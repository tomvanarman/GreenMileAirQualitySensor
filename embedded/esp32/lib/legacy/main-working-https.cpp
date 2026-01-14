#include <Arduino.h>
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "utilities.h"

XPowersPMU PMU;

#define TINY_GSM_RX_BUFFER 1024

#define TINY_GSM_MODEM_SIM7080
#include <TinyGsmClient.h>

#define SerialAT Serial1
TinyGsm modem(SerialAT);

const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register "
    "to. The GPRS service is disabled, the UE is allowed to attach for GPRS if "
    "requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an "
    "operator to register to. The GPRS service is enabled, but an allowable "
    "PLMN is currently not available. The UE will start a GPRS attach as soon "
    "as an allowable PLMN is available.",
    "Registration denied, the GPRS service is disabled, the UE is not allowed "
    "to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

enum
{
  MODEM_CATM = 1,
  MODEM_NB_IOT,
  MODEM_CATM_NBIOT,
};

const char *apn = "iot.1nce.net";

bool isConnect()
{
  modem.sendAT("+SMSTATE?");
  if (modem.waitResponse("+SMSTATE: "))
  {
    String res = modem.stream.readStringUntil('\r');
    return res.toInt();
  }
  return false;
}

void readModemResponse(uint32_t timeout)
{
  unsigned long start = millis();
  while (millis() - start < timeout)
  {
    while (Serial1.available())
    {
      int c = Serial1.read();
      Serial.write(c);
      start = millis();
    }
  }
  Serial.println();
}

void logModemStatus()
{
  Serial.println("========== SSL & MQTT Status ==========");

  modem.sendAT("+CSSLCFG?");
  readModemResponse(5000);
  modem.sendAT("+SMSTATE?");
  readModemResponse(5000);

  Serial.println("=======================================");
}

void setup()
{
  Serial.begin(115200);

  // Start while waiting for Serial monitoring
  while (!Serial)
    ;

  delay(3000);

  Serial.println();

  /*********************************
   *  step 1 : Initialize power chip,
   *  turn on modem and gps antenna power channel
   ***********************************/
  if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL))
  {
    Serial.println("Failed to initialize power.....");
    while (1)
    {
      delay(5000);
    }
  }

  // If it is a power cycle, turn off the modem power. Then restart it
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED)
  {
    PMU.disableDC3();
    // Wait a minute
    delay(200);
  }

  // Set the working voltage of the modem, please do not modify the parameters
  PMU.setDC3Voltage(3000); // SIM7080 Modem main power channel 2700~ 3400V
  PMU.enableDC3();

  // Modem GPS Power channel
  PMU.setBLDO2Voltage(3300);
  PMU.enableBLDO2(); // The antenna power must be turned on to use the GPS
  // function

  // TS Pin detection must be disable, otherwise it cannot be charged
  PMU.disableTSPinMeasure();

  /*********************************
   * step 2 : start modem
   ***********************************/

  Serial1.begin(115200, SERIAL_8N1, BOARD_MODEM_RXD_PIN, BOARD_MODEM_TXD_PIN);

  pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
  pinMode(BOARD_MODEM_RI_PIN, INPUT);

  int retry = 0;
  while (!modem.testAT(1000))
  {
    Serial.print(".");
    if (retry++ > 6)
    {
      // Pull down PWRKEY for more than 1 second according to manual
      // requirements
      digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
      delay(100);
      digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
      delay(1000);
      digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
      retry = 0;
      Serial.println("Retry start modem .");
    }
  }
  Serial.println();
  Serial.print("Modem started!");

  /*********************************
   * step 3 : Check if the SIM card is inserted
   ***********************************/

  if (modem.getSimStatus() != SIM_READY)
  {
    Serial.println("SIM Card is not inserted!!!");
    return;
  }

  // Disable RF
  modem.sendAT("+CFUN=0");
  if (modem.waitResponse(20000UL) != 1)
  {
    Serial.println("Disable RF Failed!");
  }

  /*********************************
   * step 4 : Set the network mode to NB-IOT
   ***********************************/

  modem.setNetworkMode(2); // use automatic
  modem.setPreferredMode(MODEM_NB_IOT);

  // Set the APN manually. Some operators need to set APN first when registering the network.
  modem.sendAT("+CGDCONT=1,\"IP\",\"iot.1nce.net\"");
  if (modem.waitResponse() != 1)
  {
    Serial.println("Set operators apn Failed!");
    return;
  }

  // Enable RF
  modem.sendAT("+CFUN=1");
  if (modem.waitResponse(20000UL) != 1)
  {
    Serial.println("Enable RF Failed!");
  }

  /*********************************
   * step 5 : Wait for the network registration to succeed
   ***********************************/
  SIM70xxRegStatus s;
  do
  {
    s = modem.getRegistrationStatus();
    if (s != REG_OK_HOME && s != REG_OK_ROAMING)
    {
      Serial.print(".");
      delay(1000);
    }

  } while (s != REG_OK_HOME && s != REG_OK_ROAMING);

  Serial.println();
  Serial.print("Network register info:");
  Serial.println(register_info[s]);

  modem.sendAT("+CNCFG=0,1,\"iot.1nce.net\"");
  if (modem.waitResponse() != 1)
  {
    Serial.println("Config apn Failed!");
    return;
  }

  modem.sendAT("+CNACT=0,1");
  if (modem.waitResponse() != 1)
  {
    Serial.println("Activate network bearer Failed!");
    return;
  }

  bool res = modem.isGprsConnected();
  Serial.print("GPRS status:");
  Serial.println(res ? "connected" : "not connected");

  modem.sendAT("+CNACT?");
  readModemResponse(5000);
  
  // Enable Local Time Stamp for getting network time
  modem.sendAT(GF("+CLTS=1"));
  if (modem.waitResponse(10000L) != 1)
  {
    Serial.println("Enable Local Time Stamp Failed!");
    return;
  }

  // Before connecting, you need to confirm that the time has been synchronized.
  modem.sendAT("+CCLK?");
  readModemResponse(5000);

  /*********************************
   * Simple HTTPS GET request
   *********************************/

  Serial.println("Starting HTTPS GET request...");

  modem.sendAT("+CSSLCFG=\"sslversion\",1,3");
  modem.waitResponse();

  modem.sendAT("+CSSLCFG=\"sni\",1,\"cmb.blockleven.nl\"");
  modem.waitResponse();

  modem.sendAT("+SHSSL=1,\"\"");
  modem.waitResponse();

  modem.sendAT("+SHCONF=\"URL\",\"https://cmb.blockleven.nl\"");
  modem.waitResponse();

  modem.sendAT("+SHCONF=\"BODYLEN\",1024");
  modem.waitResponse();

  modem.sendAT("+SHCONF=\"HEADERLEN\",350");
  modem.waitResponse();

  // Connect HTTPS
  modem.sendAT("+SHCONN");
  readModemResponse(15000);

  // Optional state check
  modem.sendAT("+SHSTATE?");
  readModemResponse(2000);

  // Clear headers
  modem.sendAT("+SHCHEAD");
  modem.waitResponse();

  // Add standard headers
  modem.sendAT("+SHAHEAD=\"User-Agent\",\"curl/7.47.0\"");
  modem.waitResponse();

  modem.sendAT("+SHAHEAD=\"Cache-control\",\"no-cache\"");
  modem.waitResponse();

  modem.sendAT("+SHAHEAD=\"Connection\",\"keep-alive\"");
  modem.waitResponse();

  modem.sendAT("+SHAHEAD=\"Accept\",\"*/*\"");
  modem.waitResponse();

  // Perform GET request to root path
  modem.sendAT("+SHREQ=\"/\",1");
  if (modem.waitResponse(20000) != 1)
  {
    Serial.println("HTTPS GET failed");
    return;
  }

  // Example response expected:
  // +SHREQ: "GET",200,<len>

  // Read response data (increase length if needed)
  modem.sendAT("+SHREAD=0,1024");
  readModemResponse(8000);

  // Disconnect
  modem.sendAT("+SHDISC");
  modem.waitResponse();

  Serial.println("HTTPS GET completed");
}

void loop()
{
  while (1)
  {
    while (Serial1.available())
    {
      Serial.write(Serial1.read());
    }
    while (Serial.available())
    {
      Serial1.write(Serial.read());
    }
  }

  if (!isConnect())
  {
    Serial.println("MQTT Client disconnect!");
    delay(1000);
    return;
  }
}