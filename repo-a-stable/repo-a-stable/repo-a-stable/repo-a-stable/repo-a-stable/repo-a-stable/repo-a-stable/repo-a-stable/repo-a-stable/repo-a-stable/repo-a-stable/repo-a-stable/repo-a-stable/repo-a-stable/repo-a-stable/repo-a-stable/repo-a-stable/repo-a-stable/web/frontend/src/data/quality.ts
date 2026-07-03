import { makeFetch } from 'src/util/fetch';

export const QualityController = {
    async getQualityData(deviceId: string) {
        const params = new URLSearchParams();
        params.append('deviceId', deviceId);

        const response = await makeFetch('/api/public/current-quality?' + params.toString(), {
            method: 'GET',
        });
        return response.json();
    },
};
