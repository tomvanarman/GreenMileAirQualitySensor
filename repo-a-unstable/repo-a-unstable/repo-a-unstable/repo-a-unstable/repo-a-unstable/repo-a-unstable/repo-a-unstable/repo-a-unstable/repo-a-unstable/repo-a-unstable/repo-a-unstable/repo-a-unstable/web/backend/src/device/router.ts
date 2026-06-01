import express from 'express';
import { sessionMiddleware } from '../session';
import { authMiddleware } from '../auth/user';
import DeviceModel from './device';
import { deriveKeyHex } from '../crypto';
import { createId } from '@paralleldrive/cuid2';
import QRCode from 'qrcode';

export const deviceRouter = express.Router();
deviceRouter.use(sessionMiddleware);
deviceRouter.use(authMiddleware);

/**
 * Register a new device.
 * Returns a JSON response with the device id and the secret key. This key is stored
 * in the database and can only be retrieved once.
 */
deviceRouter.post('/register', async (req, res) => {
    const { name } = req.body;

    if (!name) {
        return res.status(400).json({
            error: 'Device name is required',
        });
    }

    const deviceId = createId();
    const secret = deriveKeyHex(deviceId);

    const result = await DeviceModel.createDevice(deviceId, name, secret);
    if (result === null) {
        return res.status(500).json({
            error: 'Could not create device',
        });
    }

    const row = await DeviceModel.getById(deviceId);

    return res.json({
        ...row,
        secret,
        success: true,
    });
});

/**
 *  Revoke a device.
 */
deviceRouter.post('/revoke', async (req, res) => {
    const { deviceId } = req.body;

    if (!deviceId) {
        return res.status(400).json({
            error: 'Device ID is required',
        });
    }

    const result = await DeviceModel.revokeDevice(deviceId);
    if (!result) {
        return res.status(500).json({
            error: 'Could not revoke device',
        });
    }

    return res.json({
        success: true,
    });
});

/**
 * Get all registered devices.
 */
deviceRouter.get('/', async (req, res) => {
    const devices = await DeviceModel.getDevices();

    return res.json({
        devices: devices,
    });
});

deviceRouter.get('/qr-code', async (req, res) => {
    const { deviceId } = req.query;

    if (typeof deviceId !== 'string') {
        return res.status(400).json({ error: 'deviceId query parameter is required' });
    }

    const device = await DeviceModel.getById(deviceId);
    if (!device) {
        return res.status(404).json({ error: 'Device not found' });
    }

    const url = `${process.env.VITE_BASE_URL!}?deviceId=${device.id}`;

    res.setHeader('Content-Type', 'image/png');
    await QRCode.toFileStream(res, url);
    res.end();
});
