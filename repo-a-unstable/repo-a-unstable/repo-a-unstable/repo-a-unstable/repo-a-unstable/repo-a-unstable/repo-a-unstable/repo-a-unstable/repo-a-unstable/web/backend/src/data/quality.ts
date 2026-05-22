import db from '../db';
import { SPS30Row } from './sps30';

export interface QualityRow {
    id: number;
    value: number;
    time_unix: number;
    Device_id?: string;
}

const maxPM = 250;

export default class QualityModel {
    public static async insert(row: Omit<QualityRow, 'id'>): Promise<void> {
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
    /**
     * Calculates air quality the same way as the LED strip (see `docs/hardware/led_strip.md`)
     * @param sps30 Row of SPS30 data
     */
    public static calculateFromSPS30(sps30: SPS30Row): Promise<void> {
        const normalised = Math.min(Math.max(sps30.mc_2p0 / maxPM, 0), 1);
        // round to 2 decimal places
        const quality = Number((Math.round(normalised * 100) / 100).toFixed(2));
        return QualityModel.insert({
            value: quality,
            time_unix: sps30.time_unix,
            Device_id: sps30.Device_id,
        });
    }
}
