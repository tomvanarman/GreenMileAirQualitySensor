#!/bin/bash

echo "Pulling latest code"

git pull

echo "Restarting the node and mosquitto containers"

docker compose up -d node mosquitto --force-recreate

echo "Restarting Traefik Proxy"
docker compose up -d traefik --force-recreate

echo "Done!"
