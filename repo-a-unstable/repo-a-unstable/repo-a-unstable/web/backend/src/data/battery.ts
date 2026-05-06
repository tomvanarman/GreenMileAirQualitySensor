import db from '../db';

export interface BatteryRow {
    id: number;
    level: number;
    time_unix: number;
    Device_id?: string;
}

export default class BatteryModel {
    public static async insert(row: BatteryRow) {
        try {
            await db().query(
                'insert into DeviceBattery (level, time_unix, Device_id) values (?, ?, ?);',
                [row.level, row.time_unix, row.Device_id]
            );
        } catch (error) {
            console.log('Could not insert Battery record:', error);
        }
    }
}
