/**
 * @file      ModemMqttsExample.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co.,
 * Ltd
 * @date      2022-09-16
 *
 */
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

constexpr char client_cert_key[] = R"(-----BEGIN PRIVATE KEY-----
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQCxkSeBWWaAJ7It
9ySP04kcoU2iMGAk4T/C0lFicYY496kggtTs6MTEo7vr9g0EbcwROK3esPr1B0W2
wTQ/OIyGr9NQqUWLZyHBrhJdCjM3j2kfkuu7c7LWngssEklurlOJfPcecG0E1FmU
jnFOND55tmZbTrPpePjG16AfaOJYlH+uRQVf6evjb5cU/Hc7DjHuGqKEJ4qSaDMH
TEqvE0ftRQLRRPh0K+fOXmn+Bdneu9QhVTMepjAaYtDkJocoAD//+d2Y3cW3Kgmf
Qdsgo+E1IQn6RYL6wXaH9jvQPFLSfb7PZfFNo3QbLM+zUvbGha7Nipq9Os73SuDT
lkiL/GerAgMBAAECggEAMDbMtkeTGGsu62I28EKs8TQvQ0HFaV4sFPdMl+FhY6iD
qMs9i5Jsk0QuK5Tk1fYGOgrc0yHVEqFF4ai2ShBNv+iQBU/hDUgUl2VvoQm6/HQZ
RDR9g6mXvsStHetdOwtkrWlcJEy8nVoXdMn0pBp1eFD5nANTwKpTSpKT7NNq2kgK
yqGsvGKk2hL67/qjs8UvIzQjee+mZK0OrOhFPrPzJ2HBmBl3FOReG2XD3nrpm+5I
r7QO3fSRXwWCk3mlxrg0xK7exIVxvETxUSk1Jjqzm9IcG9LvxNzKKP55LymzbrD3
Cyx4Lip3IYMtih/fNMMm6D/b+OQRt2tZdB32gmzQuQKBgQD7BFecWsgsZFJz0zKn
xo/JB+k/4FMxikBlpxH/k9xYKIwiF7ki3cIsiAN533BygoTdK25FXato+D+qXGYs
qNEV4fxYKFISUiDnWrUMg38iWY5VDAimA2PkMOmaOgU8FmZOG4/sNEjA/C5/+W+L
m4t7Ou75FYQG90sGmmc4JgcfxwKBgQC1F4rf8uKEk0Vndj75nV3BNawER9uJPCA3
OqNAfWHd3g6tOYnGRpBbrszFY6srKCOBf5+fiV8HlrzxfTQDistGaoeFMH7zdb4W
dJArQPljZeJ+LZ9BWui2nEQWP6Q6MNQpEoDZbZ5aY12tB5/tyMIv10amWTzHWHkH
L//iFX4A/QKBgQDzFa9qQRfctYKLnBlQFKAlxD2DmE+4h/P4/SQW5y/ZR9r9IRAu
zaY+zbDgExuoKPFV5bsx/ZYDX3T4G+bulMV2dZSUIufnyhUHNvZhme5Ca4vEnJS7
gEj6MMvoiQuMtvx344Uq7y8NhKnbUMlO32DiA55tJQLoZA8J4/jVjS4U/QKBgQCY
b07JIGyoaEddxd1QxLRstL8tg0c7B8+ebc+Js4Ez/32fNvXQXwmZA1T/OtyUoV7x
3PoCkuHZQ3ucU2X7LVPa3EVjSeUzDoOJ52iL4E+M6fBzgfeyTdHiFTwC9mNJUFzm
jdrEK0/GiioKQobO49UB7FZeJHPS4ZhPAoqERs8gCQKBgQCY6DNJIEK9gAt2FK6k
K+l78nhzB/Q7/tv0m8kl7qS6vHsSvEr+Jo2NxYuXP23PItIlDkg0L8agnjyBwto2
HPC0V9hr6VU193AuhL8FldFrDnmi6Hkniq01Cu5Szj/3mA/USuUjlAvbjlSkRflZ
SAS7wMWfuRJZuqdW7CyE5D48ZA==
-----END PRIVATE KEY-----)";

constexpr char client_cert[] = R"(-----BEGIN CERTIFICATE-----
MIIERzCCAi8CFE6yVmAIzz0MQk88sInqAWmbxnqWMA0GCSqGSIb3DQEBCwUAMGIx
CzAJBgNVBAYTAk5MMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl
cm5ldCBXaWRnaXRzIFB0eSBMdGQxGzAZBgNVBAMMEm1xdHQuYmxvY2tsZXZlbi5u
bDAeFw0yNTExMTMxMTI2MjBaFw0yNjExMTMxMTI2MjBaMF4xCzAJBgNVBAYTAk5M
MRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRz
IFB0eSBMdGQxFzAVBgNVBAMMDlNJTTcwODAgQ2xpZW50MIIBIjANBgkqhkiG9w0B
AQEFAAOCAQ8AMIIBCgKCAQEAsZEngVlmgCeyLfckj9OJHKFNojBgJOE/wtJRYnGG
OPepIILU7OjExKO76/YNBG3METit3rD69QdFtsE0PziMhq/TUKlFi2chwa4SXQoz
N49pH5Lru3Oy1p4LLBJJbq5TiXz3HnBtBNRZlI5xTjQ+ebZmW06z6Xj4xtegH2ji
WJR/rkUFX+nr42+XFPx3Ow4x7hqihCeKkmgzB0xKrxNH7UUC0UT4dCvnzl5p/gXZ
3rvUIVUzHqYwGmLQ5CaHKAA///ndmN3FtyoJn0HbIKPhNSEJ+kWC+sF2h/Y70DxS
0n2+z2XxTaN0GyzPs1L2xoWuzYqavTrO90rg05ZIi/xnqwIDAQABMA0GCSqGSIb3
DQEBCwUAA4ICAQCy31mslYS7IhEraMEouzTT4q0o3F5yzTbzweY7p6Cs/m+V+vyO
8ZqQDEA14h2+azDfix09C9LUVp7EniHF4WubBNKemKatjF+pWYtcFxbwK5fKO97P
W8CwxJURqlGKPxDlI5tkE633j+7yvYHUQ3LgoZ15ylzu+fNq7q0aWvZs+s+/BC6S
J+actF2W5r9T4RAdOvsuhirXt8qexwdnH3hUujSUh4XypQKzhqY3DVbJHOw7mz//
XbenweXp6SvCHPaKjqJc5RLzbeA3j+2CcCoRDeq/RoOjzcTrWfl9CLHeLivGeHf2
8VpociHxtIb2SGu+P0U7hmDrAxQsOzXn0CXfiCnb6ORaKFZ/2y0XaWk0T1or/uAG
jBH3C6odCOAI1YjxitIhhqJIioXdoRkxTvdkNO5e8P9KuRPJknBhGfVBLn4kcOsr
n/NaK1oim6v+VF4v38/eHKDYkDnvqjsbO13EY78IRwPR1TRAhNUijm75zTibpJOD
M7KYjEgOltI+smmV1WEyh/2iNXcl9p/gk6+S2O1GcwUzNEB3jiYxrFQtWxCO+5uF
kIN0LXfROCsZM9siG5l7DprwMpqbAqpMR9NSGJvIyv/fjnqjkqUXCabAWtHh3cEf
ZRF2179gX1B+9yCqtXfNTXSLPZm4RF3ApHf0Fn7H/E1/U8ZyZ3DrGMayMA==
-----END CERTIFICATE-----)";

constexpr char ca_cert[] = R"(-----BEGIN CERTIFICATE-----
MIIFpTCCA42gAwIBAgIURxUfQf9nxsl2OQe/a3ggDRY7A00wDQYJKoZIhvcNAQEL
BQAwYjELMAkGA1UEBhMCTkwxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM
GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEbMBkGA1UEAwwSbXF0dC5ibG9ja2xl
dmVuLm5sMB4XDTI1MTExMzExMjMzMloXDTM1MTExMTExMjMzMlowYjELMAkGA1UE
BhMCTkwxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0IFdp
ZGdpdHMgUHR5IEx0ZDEbMBkGA1UEAwwSbXF0dC5ibG9ja2xldmVuLm5sMIICIjAN
BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAt81kbTQgcK5WQ+3tHG2kW50FdTjG
L3GTPKDz98v+UGO8KrRdzBER+lxM8D1s15+d+mRp/Vl4D7fDmJKYm9AEDzvvHA0O
Mc3drcXmqijqJ7gaKBr1X0wowXpVqwLeqGUaDANKsegOvMioKmEOI+xrUyhQ4MZd
varDxx2DN+DwytoQJGk4AXmRf0+dDkInx3hQBG+hszbfrkJIzpL8QObHsbpKFq67
bwByCFjtPbfdo4CpcR8ZjCMz0arvacPYCm2HbAkQ+EgcB8DRku3GeUbPGdFyJ8xu
rbYlZYBox5F4kIszQ7Ht+JGvoOd5L7zpX/6Ooe9wdyWpwWEay9KfqetTImIkrJfA
6wBd9MEyYmk+ImV/mLIYlegW+imHsxWJUI8DZ9CDn80n7SseXom3MajgQQeDy/9a
hkGW+j7CkJS4VQYhvB6aaLs+To77/HcA9O588UjBG+jAEJQvu0yax+gQ8AYTokH2
URxwp65Rw7oGzYo4CkxVfyThEgmvntds/q7jw827UAJBRVcD96UL1/MFT9PEeQ7F
1O69jSnm7CpJRktt4/kgsT+aLXClhNVyIZr20NBEK3eTgD/O/DI4D7sYgMYoCG5j
2+4Fp6QTaCKnz540caIDfyOS43yCUFTb2nb1U/Oz162KkIU6I0ZUHgu1ziXnWP9X
cEJgkjRt/8HxEVECAwEAAaNTMFEwHQYDVR0OBBYEFJIe55MbB88XFWxVVoTbwhYl
HwMJMB8GA1UdIwQYMBaAFJIe55MbB88XFWxVVoTbwhYlHwMJMA8GA1UdEwEB/wQF
MAMBAf8wDQYJKoZIhvcNAQELBQADggIBAH6A3hkYMVMxJFtVCBYjgWBtppNY2BSm
ZLWx6LGbVh8AXC6rr3Rsevv3yKuQAvo3uA60vMTzHz9anTCU33BgLrhKd29Tkd6q
UE3EqOAiejmPhTm1HnS2n7/fmqsY0e5rDKd9hdOCi8UfLJ1ZksruODm91Cu5BIu4
8++Epq6Gr64AGMjYH6+8alXYc5lBz9SWM2mTSU/EqjFE6yIeer4YE2DOm2Vt5CwX
EkJB0NByEMbzeVroPME+ka5nsDE4mPYXkq3B066SDLdODED57K5alwdBf3cOMMQa
Rb6SPyFFBxFfnaaow6z4f7YY6Y7UXVTIrnLnixxjcqz+jqCLF7j9q/kEpAgswmEl
aH97YMFgd8SgYk/PO47HXfTrygxynHUFpWR5Gc4EcuuQSy+QxJRujbdfHnN7KmeW
NeAyAdI2iB7Fhr9M7wza2D/vc2wtaWpD1Nh6YnY9MPLRHAhE0glh6XMdTHUmrOTW
1hkRlY207hOzTpUoV7xJGrN04vkAxag61Jc50xY6IfFgRorH0iU7FeMxF6rLo28e
wDI64+pLg5h5JCVynF4nHOz2d64Ja4WN4B3i9pQfVKisKnpsK4DVZW5+1vYfMIRB
1Q7iIjEVObBinNCGwHwAvDfZxlo3Rno4kHA/2e0MEJcts47W+KQFbrWLA7e1MJlv
JLx5iCiOSB92
-----END CERTIFICATE-----)";

enum
{
  MODEM_CATM = 1,
  MODEM_NB_IOT,
  MODEM_CATM_NBIOT,
};

const char *apn = "iot.1nce.net";

// server address and port
const char server[] = "mqtt.blockleven.nl";
const int port = 443;

char clientID[] = "client";

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

void writeCaFiles(int index, const char *filename, const char *data,
                  size_t length)
{
  modem.sendAT("+CFSTERM");
  modem.waitResponse();

  modem.sendAT("+CFSINIT");
  if (modem.waitResponse() != 1)
  {
    Serial.println("INITFS FAILED");
    return;
  }
  // AT+CFSWFILE=<index>,<filename>,<mode>,<filesize>,<input time>
  // <index>
  //      Directory of AP filesystem:
  //      0 "/custapp/" 1 "/fota/" 2 "/datatx/" 3 "/customer/"
  // <mode>
  //      0 If the file already existed, write the data at the beginning of the
  //      file. 1 If the file already existed, add the data at the end o
  // <file size>
  //      File size should be less than 10240 bytes. <input time> Millisecond,
  //      should send file during this period or you can’t send file when
  //      timeout. The value should be less
  // <input time> Millisecond, should send file during this period or you can’t
  // send file when timeout. The value should be less than 10000 ms.

  size_t payloadLength = length;
  size_t totalSize = payloadLength;
  size_t alardyWrite = 0;

  while (totalSize > 0)
  {
    size_t writeSize = totalSize > 10000 ? 10000 : totalSize;

    modem.sendAT("+CFSWFILE=", index, ",", "\"", filename, "\"", ",",
                 "0", ",", writeSize, ",", 10000);
    modem.waitResponse(30000UL, "DOWNLOAD");
  REWRITE:
    modem.stream.write(data + alardyWrite, writeSize);
    if (modem.waitResponse(30000UL) == 1)
    {
      alardyWrite += writeSize;
      totalSize -= writeSize;
      Serial.printf("Writing:%d overage:%d\n", writeSize, totalSize);
    }
    else
    {
      Serial.println("Write failed!");
      delay(1000);
      goto REWRITE;
    }
  }

  Serial.println("Wirte done!!!");

  modem.sendAT("+CFSTERM");
  if (modem.waitResponse() != 1)
  {
    Serial.println("CFSTERM FAILED");
    return;
  }
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

  // modem.sendAT("+CGATT?");
  // readModemResponse(5000);

  // modem.sendAT("+CCGNAPN");
  // readModemResponse(5000);

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

  // Step 1: configure SSL for MQTT
  modem.sendAT("+CSSLCFG=\"sslversion\",1,3"); // TLS 1.2
  modem.waitResponse();
  modem.sendAT("+CSSLCFG=\"sni\",1,\"mqtt.blockleven.nl\"");
  modem.waitResponse();
  modem.sendAT("+SMCONF=\"SSL\",1"); // bind SSL context 1 to MQTT
  modem.waitResponse();

  // Write the CA root
  writeCaFiles(3, "ca.crt", ca_cert, strlen(ca_cert));
  // Write the client certificate
  writeCaFiles(3, "client.crt", client_cert, strlen(client_cert));
  // Write the client key
  writeCaFiles(3, "client.key", client_cert_key, strlen(client_cert_key));

  modem.sendAT("+CSSLCFG=\"CONVERT\",2,\"ca.crt\"");
  readModemResponse(5000);

  modem.sendAT("+CSSLCFG=\"CONVERT\",1,\"client.crt\",\"client.key\"");
  readModemResponse(5000);

  modem.sendAT("+SMSSL=1,\"ca.crt\",\"client.crt\"");
  readModemResponse(5000);

  // Step 2: MQTT settings
  modem.sendAT("+SMCONF=\"URL\",\"mqtt.blockleven.nl\",443");
  modem.waitResponse();
  modem.sendAT("+SMCONF=\"CLIENTID\",\"ESP32Client\"");
  modem.waitResponse();
  modem.sendAT("+SMCONF=\"KEEPTIME\",60");
  modem.waitResponse();
  modem.sendAT("+SMCONF=\"CLEANSS\",1");
  modem.waitResponse();

  // Step 3: connect
  modem.sendAT("+SMCONN");
  int8_t ret = modem.waitResponse(30000);
  Serial.print("MQTT connect result:");
  Serial.println(ret == 1 ? "success" : "failed");
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