import db from '../db';

export interface SHT41Row {
    id: number;
    temperature: number;
    humidity: number;
    time_unix: number;
    Device_id?: string;
}

export default class SHT41Model {
    public static async insert(row: SHT41Row) {
        try {
            await db().query(
                'insert into SHT41 (temperature, humidity, time_unix, Device_id) values (?, ?, ?, ?);',
                [row.temperature, row.humidity, row.time_unix, row.Device_id]
            );
        } catch (error) {
            console.log('Could not insert SHT41 record:', error);
        }
    }
}
