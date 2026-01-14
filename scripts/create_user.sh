#!/bin/bash

username=$1
password=$2

echo "Creating user with username: $username"

docker compose exec -T node /bin/sh -c "npx tsx scripts/createUser.ts $username $password"

echo "User created."
