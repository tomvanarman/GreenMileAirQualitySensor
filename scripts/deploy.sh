#!/bin/bash

echo "Pulling latest code"

git pull

chmod 600 docker/config/traefik/acme.json

echo "Building docker compose services"
docker compose build
echo "Starting docker compose services"
docker compose up -d
