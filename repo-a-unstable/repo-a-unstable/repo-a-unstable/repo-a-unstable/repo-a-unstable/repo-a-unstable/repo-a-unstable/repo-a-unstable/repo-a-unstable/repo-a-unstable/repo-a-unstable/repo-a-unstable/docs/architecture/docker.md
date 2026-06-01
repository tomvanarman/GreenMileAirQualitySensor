# Docker

The entire project is containerized using Docker. This means that all services run in isolated
containers. This makes it easy to deploy and update the services.

See the [System Architecture](/architecture/system-architecture) for an overview of all services.

## Configuration

All services are configured using environment variables. See
[Environment Variables](/security/environment-variables) for a list of all environment variables.

All docker configuration files are located in the `docker` directory.

-   `grafana`: Contains the grafana config file
-   `traefik`: Contains the traeffik config files and an `acme.json` file for storing SSL
    certificates

## Services

### Traeffik

Traeffik is used as a reverse proxy and load balancer. It is the only service that is exposed
outside of the docker network. See [network & SSL](/server/network-ssl) for more information.

### Grafana

We run a self-hosted instance of Grafana to visualize the data collected by the backend API.

### Database

We use MariaDB as our database. It runs in its own container and is not exposed outside of the
docker network. The backend API and Grafana connect to the database using the internal docker
network.

### Backend API

The backend API is a NodeJS application that collects data from the devices and stores it in the
database.

### Frontend Application

The frontend application is a ReactJS app build as a static SPA (Single Page Application). It is
served using Nginx.
