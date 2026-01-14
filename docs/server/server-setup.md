# Server Setup

Since all services use Docker, this project can run on an OS that runs docker.

## Deploying & Updating

The application code lives in `~/cmb`. A deploy script can be used to start all services the first
time.

### Environment Variables

But first the environment variables need to be set. See [security](/security/environment-variables).

### Deploying

```bash
bash scripts/deploy.sh
```

This script will build the docker compose services, restart the containers that have changed, and
then reloads nginx. After the script is done it might take a minute or two for the database and
Grafana to finish loading.
