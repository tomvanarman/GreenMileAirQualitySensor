#!/bin/bash

echo "Pulling latest code"

git pull

echo "Building docker compose services"
docker compose build
echo "Starting docker compose services"
docker compose up -d

docker compose restart nginx
