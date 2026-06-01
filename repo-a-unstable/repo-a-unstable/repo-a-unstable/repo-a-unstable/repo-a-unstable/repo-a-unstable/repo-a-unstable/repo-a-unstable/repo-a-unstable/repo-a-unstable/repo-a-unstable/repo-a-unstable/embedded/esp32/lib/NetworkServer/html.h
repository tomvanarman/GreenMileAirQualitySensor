#pragma once
#include <Arduino.h>
const String index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Climate measuring box setup</title>
    <style>
        :root {
            color-scheme: light dark;
            --bg: linear-gradient(145deg, #312e81, #0f172a);
            --card-bg: rgba(255, 255, 255, 0.1);
            --card-border: rgba(255, 255, 255, 0.25);
            --input-bg: rgba(255, 255, 255, 0.15);
            --input-border: rgba(255, 255, 255, 0.3);
        }
        * {
            box-sizing: border-box;
            font-family: "Inter", system-ui, -apple-system, BlinkMacSystemFont, sans-serif;
        }
        body {
            margin: 0;
            min-height: 100vh;
            display: grid;
            place-items: center;
            background: var(--bg);
            color: #f8fafc;
        }
        .card {
            width: min(420px, 90vw);
            padding: 2.5rem 2rem;
            backdrop-filter: blur(14px);
            background: var(--card-bg);
            border: 1px solid var(--card-border);
            border-radius: 18px;
            box-shadow: 0 24px 48px rgba(15, 23, 42, 0.35);
        }
        h1 {
            margin: 0 0 1.5rem;
            font-size: 1.75rem;
            font-weight: 600;
            text-align: center;
            letter-spacing: 0.02em;
        }
        form {
            display: grid;
            gap: 1rem;
        }
        label {
            font-size: 0.95rem;
            font-weight: 500;
            letter-spacing: 0.01em;
        }
        input {
            margin-top: 0.45rem;
            padding: 0.75rem;
            width: 100%;
            border-radius: 12px;
            border: 1px solid var(--input-border);
            background: var(--input-bg);
            color: inherit;
            outline: none;
            transition: border-color 0.2s ease, transform 0.2s ease;
        }
        input:focus {
            border-color: rgba(129, 140, 248, 0.9);
            transform: translateY(-1px);
        }
        button {
            margin-top: 0.75rem;
            padding: 0.85rem;
            border: none;
            border-radius: 12px;
            font-size: 1rem;
            font-weight: 600;
            background: rgba(129, 140, 248, 0.95);
            color: #0f172a;
            cursor: pointer;
            transition: transform 0.2s ease, box-shadow 0.2s ease;
        }
        button:hover {
            transform: translateY(-1px);
            box-shadow: 0 12px 24px rgba(129, 140, 248, 0.35);
        }
        .helper {
            margin-top: 1rem;
            font-size: 0.85rem;
            text-align: center;
            opacity: 0.75;
        }
    </style>
</head>
<body>
    <section class="card">
        <h1>Climate measuring box setup</h1>
        <form action="/save" method="post">
            <label>
                SSID
                <input name="ssid" placeholder="Network name" required />
            </label>
            <label>
                Password
                <input name="pass" type="password" placeholder="Network password" required />
            </label>
            <label>
                Device ID
                <input name="device_id" placeholder="Your device identifier" required />
            </label>
            <label>
                Device Key
                <input name="device_key" type="password" placeholder="Your device key" required />
            </label>
            <button type="submit">Save &amp; Connect</button>
        </form>
        <p class="helper">Enter your network credentials to bring the ESP32 online.</p>
    </section>
</body>
</html>
)rawliteral";
