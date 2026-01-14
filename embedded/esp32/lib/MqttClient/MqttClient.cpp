#include "MqttClient.h"

#include "DEBUG.h"

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

void MqttClient::Connect()
{
  // Ensure WiFi is connected before attempting MQTT connection
  if (!WiFi.isConnected())
  {
    DEBUG_WARN("WiFi not connected. Cannot establish MQTT connection.");
    return;
  }

  DEBUG_SECTION("MQTT Connect");

  // String server = credential_manager_.GetMqttServer();
  // int port = credential_manager_.GetMqttPort();

  String server = "mqtt.blockleven.nl";
  int port = 443;

  DEBUG_KV("MQTT Server", server);
  DEBUG_KV("MQTT Port", port);

  esp_client_.setCACert(ca_cert);
  esp_client_.setCertificate(client_cert);
  esp_client_.setPrivateKey(client_cert_key);

  client_.setServer(server.c_str(), port);

  // Only attempt connection if not already connected
  if (!client_.connected())
  {
    DEBUG_INFO("Attempting MQTT connection...");

    // Use a fixed client ID for this ESP32 device
    if (client_.connect("ESP32Client"))
    {
      DEBUG_OK("MQTT connected");
    }
    else
    {
      // Log the specific error code for debugging
      DEBUG_FAIL("MQTT connection failed, rc=" + String(client_.state()));
    }
  }
}

void MqttClient::Disconnect()
{
  // Only disconnect if currently connected to avoid unnecessary operations
  if (client_.connected())
  {
    client_.disconnect();
    DEBUG_INFO("MQTT disconnected");
  }
}

void MqttClient::Reconnect()
{
  // Check WiFi connectivity before attempting MQTT reconnection
  if (!WiFi.isConnected())
  {
    DEBUG_WARN("WiFi not connected. Cannot reconnect MQTT.");
    return;
  }

  // Delegate to Connect() method which handles connection logic
  Connect();
}

void MqttClient::Publish(const char *topic, const String &payload)
{
  if (client_.connected())
  {
    // Attempt to publish and capture success status
    bool success = client_.publish(topic, payload.c_str());
    if (success)
    {
      DEBUG_OK("Published to topic: " + String(topic));
    }
    else
    {
      // Publication can fail due to buffer full, network issues, etc.
      DEBUG_FAIL("Failed to publish to topic: " + String(topic));
    }
  }
  else
  {
    // Fail silently but log the attempt for debugging
    DEBUG_WARN("MQTT not connected. Cannot publish.");
  }
}

bool MqttClient::IsConnected()
{
  // Delegate to underlying PubSubClient for connection status
  return client_.connected();
}

void MqttClient::Loop()
{
  // Process incoming MQTT messages and maintain keep-alive
  // This must be called regularly for proper MQTT operation
  client_.loop();
}
