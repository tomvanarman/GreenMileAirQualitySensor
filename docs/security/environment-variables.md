# Environment Variables

Some env variables are secrets and are marked as **secret**. See the next section for a definition
of a secret env variable.

## Database

- `MARIADB_PORT`: MariaDB service port, defaults to `3306`
- `MARIADB_ROOT_PASSWORD`: **secret** Root password for database.
- `MARIADB_USER`: Username of the database
- `MARIADB_PASSWORD`: **secret** Password of `MARIADB_USER`
- `MARIADB_DATABASE`: Name of the database
- `MARIADB_HOST`: Database host

## Web / API

- `CRYPTO_SECRET`: **secret** Secret used for cryptographic operations
- `VITE_BASE_URL`: Base url of domain, e.g. `https://example.com/`
- `VITE_API_BASE_URL`: URL of the API, e.g. `https://api.example.com/`

## Grafana

- `GRAFANA_PORT`: Port where Grafana listens to
- `GRAFANA_USER`: Grafana username
- `GRAFANA_PASSWORD`: **secret** Grafana password

## Generating Secure Secrets

Every secret (password) should be a _cryptographically_ strong 32 byte hex string. An example would
be `aa19eed475700617f31d41ba4ccc2a57a501e314700f93fbb9d8c4e24b7970ed`

If you are on linux you can generate one with the following command:

```
openssl rand -hex 32
```
