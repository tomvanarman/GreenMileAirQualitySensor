import express from 'express';

import QualityModel from '../data/quality';

export const publicRouter = express.Router();
publicRouter.get('/current-quality', async (req, res) => {
    const deviceId = req.query.deviceId;
    if (typeof deviceId !== 'string') {
        return res.status(400).json({ error: 'deviceId query parameter is required' });
    }

    const latestQuality = await QualityModel.getLatest(deviceId);
    if (latestQuality) {
        return res.json({ quality: latestQuality.value });
    } else {
        return res.json({ error: 'No measurements yet' });
    }
});
