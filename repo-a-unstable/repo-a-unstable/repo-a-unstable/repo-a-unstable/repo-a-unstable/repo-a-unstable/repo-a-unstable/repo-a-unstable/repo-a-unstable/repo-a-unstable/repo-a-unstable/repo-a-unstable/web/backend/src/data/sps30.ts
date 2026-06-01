import db from '../db';

export interface SPS30Row {
    id: number;
    mc_1p0: number;
    mc_2p0: number;
    mc_4p0: number;
    mc_10p0: number;
    nc_0p5: number;
    nc_1p0: number;
    nc_2p5: number;
    nc_4p0: number;
    nc_10p0: number;
    typical_particle_size: number;
    time_unix: number;
    Device_id?: string;
}

export default class SPS30Model {
    public static async insert(row: SPS30Row) {
        try {
            await db().query(
                'insert into SPS30 (mc_1p0, mc_2p0, mc_4p0, mc_10p0, nc_0p5, nc_1p0, nc_2p5, nc_4p0, nc_10p0, typical_particle_size, time_unix, Device_id) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);',
                [
                    row.mc_1p0,
                    row.mc_2p0,
                    row.mc_4p0,
                    row.mc_10p0,
                    row.nc_0p5,
                    row.nc_1p0,
                    row.nc_2p5,
                    row.nc_4p0,
                    row.nc_10p0,
                    row.typical_particle_size,
                    row.time_unix,
                    row.Device_id,
                ]
            );
        } catch (error) {
            console.log('Could not insert SPS30 record:', error);
        }
    }
}
