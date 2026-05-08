import './env';

import express from 'express';
import bodyParser from 'body-parser';
import db from './db';
import cors from 'cors';

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

app.use('/api/public', publicRouter);
app.use('/api/user', userRouter);
app.use('/api/devices', deviceRouter);
app.use('/api/data', dataRouter);

app.listen(3000, () => {
    if (!IS_PRODUCTION) console.log('Listening on http://localhost:3000!');
});
