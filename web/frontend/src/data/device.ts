import { makeFetch } from 'src/util/fetch';

export type DeviceRow = {
    id: string;
    name: string;
    updated_at: string;
    level?: number;
};

export const DeviceController = {
    async getDevices(): Promise<{ devices: DeviceRow[] }> {
        const response = await makeFetch('/api/devices');
        return response.json();
    },
    async registerDevice(name: string): Promise<DeviceRow & { secret: string }> {
        const response = await makeFetch('/api/devices/register', {
            method: 'POST',
            body: JSON.stringify({ name }),
            headers: {
                'Content-Type': 'application/json',
            },
            credentials: 'include',
        });
        return response.json();
    },
    async revokeDevice(deviceId: string): Promise<void> {
        await makeFetch('/api/devices/revoke', {
            method: 'POST',
            body: JSON.stringify({ deviceId }),
            headers: {
                'Content-Type': 'application/json',
            },
            credentials: 'include',
        });
    },
    async getQRCode(deviceId: string): Promise<Response> {
        return makeFetch(`/api/devices/qr-code?deviceId=${deviceId}`, {
            method: 'GET',
            credentials: 'include',
        });
    },
};
