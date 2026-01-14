### Issue board ideeenbus (buiten MVP):

#### - Sensors:

1. SHT41(Humidity, temperature sensor) [Adafruit Sensirion]
2. SPS30 (fine particles) [Sensirion]

- (CO2 sensor) [SCD41]
- (VOC sensor) [SGP41/BME688]
- (NO2 sensor) [SPEC/GRAVITY brand]
- (Temperature, humidity sensor with water resistant enclosure) [SHT31]

#### - User interface:

1. Grafana webApp

#### - Storage:

1. Back end

#### - Networking:

1. 4G cellular

#### - Power:

1. 3.2V 230Ah 736Wh LiFePO4 (Grade A battery) [Eve LF230] 8-9kg!!

- Solar panel, Solar panel cleaner? DIY solar panel?

```
- Assuming a 736 Wh LiFePO₄ battery and continuous power usage between 1 W and 5 W, the battery can realistically power the system for approximately 6 to 31 days without recharge. This range corresponds to the lower end of 1 W, which gives the longest runtime (~31 days), and the higher end of 5 W, which gives the shortest (~6 days). Limiting the depth-of-discharge to 80 % slightly reduces these runtimes. Duty-cycling sensors or putting the ESP32 into deep sleep will further extend battery life, while occasional spikes above 5 W (e.g., 6 W) would reduce runtime proportionally.
```

#### - Target State-of-Charge (SoC) window:

##### - **Upper limit:** 90 % SoC

    - Avoids full voltage stress on LiFePO₄ cells

##### - **Lower limit:** 80 % SoC

    - Avoids deep discharge, keeps stress minimal

**Ideal working range is 80–90 % SoC** — a shallow cycle of ~10 %
