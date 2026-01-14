import db from '../db';

export interface QualityRow {
    id: number;
    value: number;
    time_unix: number;
    Device_id?: string;
}

export default class QualityModel {
    public static async insert(row: QualityRow) {
        try {
            await db().query(
                'insert into Quality (value, time_unix, Device_id) values (?, ?, ?);',
                [row.value, row.time_unix, row.Device_id]
            );
        } catch (error) {
            console.log('Could not insert Quality record:', error);
        }
    }

    public static async getLatest(deviceId: string): Promise<QualityRow | null> {
        try {
            const rows = await db().query(
                'select value, time_unix from Quality where Device_id = ? order by time_unix desc limit 1;',
                [deviceId]
            );
            if (rows.length) return rows[0];
            return null;
        } catch (error) {
            console.log('Could not fetch latest Quality record:', error);
            return null;
        }
    }
}
