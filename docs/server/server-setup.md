# Server Setup

Since all services use Docker, this project can run on an OS that runs docker.

## Server Specs

We use a raspberry pi zero with 1GB of ram and it runs on `6.12.47+rpt-rpi-v8` Raspberry Pi OS
server: 6.12.47.

## Deploying & Updating

The application code lives in `~/cmb`. A deploy script can be used to start all services the first
time.

### Environment Variables

But first the environment variables need to be set. See [security](/security/environment-variables).

### Deploying

```bash
cd cmb
chmod +x scripts/deploy.sh
./deploy.sh
```

### Updating

To update the code and services the `update.sh` script needs to be run.

```bash
cd cmb
chmod +x scripts/update.sh
./update.sh
```
