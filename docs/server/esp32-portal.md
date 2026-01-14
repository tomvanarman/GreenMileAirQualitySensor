# CMB portal

## Introduction
Whenever a CMB starts it starts a WiFi hotspot with a site to configure the CMB. This allows the Urban planner to easily configure the CMB.

## System overview
When the CMB starts it will first validate the configuration stored in the non-volatile memory. If the configuration fails the validation the CMB will start a WiFi hotspot with a portal to configure the CMB. The portal is hosted on the CMB and can be accessed by connecting to the WiFi hotspot and navigating to the IP address of the CMB in a web browser (often 192.168.4.1).

## Configuration
The portal allows the user to configure various settings of the CMB, including:
- WiFi settings (SSID, password)
- MQTT settings (broker address, port)

## Updating the website
The website files are located in the 'embedded/NetworkServer/html.h' file. To update the website, modify the HTML, CSS, and JavaScript code in this file and recompile the firmware. Make sure all the html is encapsulated in R"rawliteral( ... )rawliteral" for proper embedding. Where ... is the html code and is contained in a String variable.

## Accessing the portal after initial setup
To access the portal after initial setup, you need to either change the code or wipe the non-volatile memory to force the CMB to start the portal again. There is currently no way to access the portal without doing this.

