# Setup: Air Pollution Sensor Box

This guide explains how to connect your air pollution sensor box to the Green Mile system.

## Step-by-Step Instructions

1. **Upload and start the firmware**
    - Upload the code to the microcontroller.
    - Start the device.

2. **Wait for the status LED**
    - After a few seconds, the RGB LED should start blinking purple.

3. **Connect to the config Wi-Fi network**
    - Connect your laptop or phone to the created wifi network of the microcontroller:
    - **SSID:** `ESP_Config`
    - **Password:** `test1234`

4. **Open the configuration page**
    - Open your browser and go to [http://192.168.4.1](http://192.168.4.1/).

5. **Create a Green Mile account (if needed)**
    - If you do not already have an account, create one on the server.

    **Create a user via SSH:**
    - Connect to the server:
        - `ssh user@IPv4`
    - Enter the server password.
    - Go to the project directory:
        - `cd cmb`
    - Run the user creation script:
        - `bash scripts/create_user.sh "username" "password"`
    - Replace `"username"` and `"password"` with your own username and password.

6. **Register your device**
    - Open [https://greenmile.tapp.city/devices](https://greenmile.tapp.city/devices).
    - Register a new device.
    - Choose the device name with the **name convention from the guide below** this step is neccesary please do.
    - Save the generated **Device ID** and **Device Key (Secret Key)**.
    - :warning: **Important:** the Device Key is shown only once. Store it in a safe place.

7. **Enter Wi-Fi and device credentials**
    - On the configuration page, enter:
    - SSID (your Wi-Fi name)
    - Password (your Wi-Fi password)
    - Device ID
    - Device Key

    ![Login Credentials Page](../assets/LoginCredentialsPage.png)

8. **Save and connect**
    - Click **Save & Connect**.
    - The sensor box should connect to your network.
    - If the box keeps blinking, verify the credentials and try again.

9. **Verify incoming data**
    - Open [Grafana](https://grafana.greenmile.tapp.city/) or
      [Device Portal](https://greenmile.tapp.city/devices).
    - Sign in with the username and password created earlier.
    - Confirm sensor data is visible.

## Device Name Convention

Use this naming convention when registering a new device.

Format: TYPE-NNN-OWNER

- **TYPE**: WIFI or SIM (always uppercase)
- **NNN**: three-digit sequence number for the box (001, 002, ...)
- **OWNER**: short identifier of the person or team who added the box (optional, but recommended)

Examples:

- WIFI-001-Devgreenmile - first box using Wi-Fi, created by Devgreenmile
- SIM-042-Jan - box number 042 using a SIM connection, added by Jan

Rules:

- Use - as the separator.
- 1 name per device.
- Maximum length: 32 characters.
- TYPE must be uppercase: WIFI` or SIM.
- The sequence number must always have three digits.
- Check existing sensor boxes in the dashboard before choosing a new number.