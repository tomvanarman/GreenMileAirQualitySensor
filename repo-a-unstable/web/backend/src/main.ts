import './env';

import express from 'express';
import mqtt from 'mqtt';
import bodyParser from 'body-parser';
import db from './db';
import cors from 'cors';

// MQTT client setup
const mqttprotocol = 'mqtt'
const mqtthost = 'greenmile.tapp.city'
const mqttport = '1883'
const mqttconnectUrl = `${mqttprotocol}://${mqtthost}:${mqttport}`
const mqttclientId = `backendserver_mqtt_${Math.random().toString(16).slice(3)}`

export const client = mqtt.connect(mqttconnectUrl, {
  clientId: mqttclientId,
  clean: true,
  connectTimeout: 4000,
  username: 'emqx',
  password: 'public',
  reconnectPeriod: 1000,
});

import { dataRouter } from './data/router';
import { deviceRouter } from './device/router';
import { userRouter } from './user/router';
import { publicRouter } from './public/router';
import { IS_PRODUCTION } from './env';

// create db pool
db();



// backend API
const app = express();

// Set the raw body as a new field for signature verification
declare module 'express-serve-static-core' {
    interface Request {
        rawBody: Buffer;
    }
}

app.use(
    bodyParser.json({
        verify: (req, res, buf) => {
            (req as express.Request).rawBody = buf;
        },
    })
);
// cors setup
app.use(
    cors({
        origin: process.env.VITE_BASE_URL!,
        credentials: true,
    })
);

client.on('connect', () => {
    client.subscribe('climate-box/#', (err) => {
        if (err) {
            console.error('Failed to subscribe to MQTT topic:', err);
        } else {
            console.log('Subscribed to MQTT topic: climate-box/#');
        }
    });
});

client.on('error', (err) => {
    console.error('MQTT connection error:', err);
});

client.on('disconnect', () => {
    console.warn('MQTT client disconnected');
});

app.use('/api/public', publicRouter);
app.use('/api/user', userRouter);
app.use('/api/devices', deviceRouter);
app.use('/api/data', dataRouter);

app.listen(3000, () => {
    if (!IS_PRODUCTION) console.log('Listening on http://localhost:3000!');
});
