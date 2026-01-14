import express from 'express';

import SPS30Model from './sps30';
import SHT41Model from './sht41';
import QualityModel from './quality';
import DeviceModel from '../device/device';
import { verifyDeviceHmac } from '../crypto';

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
    console.log({ deviceId, bodySignature });

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
    console.log('Device HMAC valid:', isValid);
    if (!isValid) {
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

dataRouter.post('/sps30', (req, res) => {
    SPS30Model.insert(req.body);
    res.status(204).end();
});

dataRouter.post('/sht41', (req, res) => {
    SHT41Model.insert(req.body);
    res.status(204).end();
});

dataRouter.post('/quality', (req, res) => {
    QualityModel.insert(req.body);
    res.status(204).end();
});
