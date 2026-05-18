FROM node:22.20.0-alpine3.22

WORKDIR /app

# Copy package.json and package-lock.json
COPY ./package*.json .
# Install dependencies
RUN npm ci
# Copy source files
COPY . .
# Build project
RUN npm run build

CMD ["npm", "start"]