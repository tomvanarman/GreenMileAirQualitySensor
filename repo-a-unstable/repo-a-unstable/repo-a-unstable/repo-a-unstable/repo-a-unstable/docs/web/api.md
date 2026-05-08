# API

The backend is a NodeJS application that collects data from the devices and stores it in the
database.

## Api Routes

The API consists of 4 sections:

- `/api/public`: publicly accessible routes
- `/api/user`: routes for handling users (authentication 2FA etc.)
- `/api/devices`: for managing devices (requires an authenticated user)
- `/api/data`: for inserting new data points from devices

The code is structured in a similair manner.

## Security

### User Authentication

The `/api/user` and `/api/devices` routes require user authentication. Users are authenticated using
a username/password and must complete a 2FA challenge using TOTP. Once auhtenticated a session will
be stored in the browsers using cookies.

### Device Authentication

Device must identify themselves using their device ID. An id is optained from the devices dashboard
when regeristering a device [see here](frontend.md).

Each request from a device must include the device ID in the `X-Device-ID` header. On top of that
the body needs to be signed using HMAC-SHA256 with the devices secret key. The signature must be
included in the `X-Signature` header as a hex encoded string.

Appropriate HTTP status codes will be returned when authentication fails.

## Embedded API endpoints

### POST /api/data/sps30

Insert a new SPS30 measurement.

Example request body:

```json
{
    "pm1_0": 12.34,
    "pm2_5": 23.45,
    "pm4_0": 34.56,
    "pm10_0": 45.67,
    "nc0_5": 1234,
    "nc1_0": 2345,
    "nc2_5": 3456,
    "nc4_0": 4567,
    "nc10_0": 5678,
    "typical_particle_size": 1.23,
    "time_unix": 1687425600
}
```

### POST /api/data/sht41

Insert a new SHT41 measurement.

Example request body:

```json
{
    "temperature": 23.45,
    "humidity": 45.67,
    "time_unix": 1687425600
}
```
