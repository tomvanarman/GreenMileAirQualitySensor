import express from 'express';

import SPS30Model from './sps30';
import SHT41Model from './sht41';
import QualityModel from './quality';
import DeviceModel from '../device/device';
import BatteryModel from './battery';
import { verifyDeviceHmac } from '../crypto';
import { IS_PRODUCTION } from '../env';

export const dataRouter = express.Router();

/**
 * Epxress middleware to verify device authenticity.
 * It checks for the presence of `x-device-id` and `x-signature` headers,
 * verifies that the device is known and not revoked, and checks the body signature.
 */
async function verifyDevice(
    req: express.Request,
    res: express.Response,
    next: express.NextFunction
) {
    const deviceId = req.headers['x-device-id'] as string | null;
    const bodySignature = req.headers['x-signature'] as string | null;
    if (!IS_PRODUCTION) console.log({ deviceId, bodySignature });

    if (!deviceId || !bodySignature) {
        return res.status(401).json({ error: 'Missing device authentication headers' });
    }

    // verify that the device is known and not revoked
    const device = await DeviceModel.getById(deviceId);
    if (!device) {
        return res.status(401).json({ error: 'Unknown Device' });
    } else if (device.revoked) {
        return res.status(403).json({ error: 'Device revoked' });
    }
    // verify body signature
    const isValid = await verifyDeviceHmac(bodySignature, req.rawBody, device.secret_hex);
    if (!IS_PRODUCTION) console.log('Device HMAC valid:', isValid);
    if (!isValid) {
        console.error(`Invalid body signature for device ${deviceId}`);
        return res.status(401).json({ error: 'Invalid body signature' });
    }
    req.body.Device_id = deviceId;

    next();
}

dataRouter.use(verifyDevice);
//          All Routes Below Require Device Authentication
// ==============================================================================

dataRouter.post('/test', (req, res) => {
    res.json({
        success: true,
    });
});

dataRouter.post('/sps30', async (req, res) => {
    try {
        await Promise.all([SPS30Model.insert(req.body), QualityModel.calculateFromSPS30(req.body)]);
        res.status(204).end();
    } catch (error) {
        console.log('Error processing SPS30 data:', error);
        return res.status(500).end();
    }
});

dataRouter.post('/sht41', async (req, res) => {
    try {
        await SHT41Model.insert(req.body);
        res.status(204).end();
    } catch (error) {
        console.log('Error processing SHT41 data:', error);
        return res.status(500).end();
    }
});

dataRouter.post('/battery', async (req, res) => {
    try {
        await BatteryModel.insert(req.body);
        res.status(204).end();
    } catch (error) {
        console.log('Error processing Battery data:', error);
        return res.status(500).end();
    }
});

// dataRouter.post('/quality', (req, res) => {
//     QualityModel.insert(req.body);
//     res.status(204).end();
// });
