FROM node:22.20.0-alpine3.22 AS build

WORKDIR /app

# Copy package.json and package-lock.json
COPY ./package*.json .

# Install dependencies
RUN npm ci
# Copy source files
COPY . .
# Build project
RUN npm run build

FROM nginx:alpine

COPY --from=build /app/client /usr/share/nginx/html