import db from '../db';

export interface DeviceRow {
    id: string;
    name: string;
    updated_at: number;
    secret_hex: string;
    revoked: boolean;
}

export default class DeviceModel {
    /**
     * Create a new device
     * @param id
     * @param name
     * @param secretHex
     * @returns its id and `null` if the device could not be inserted
     */
    public static async createDevice(
        id: string,
        name: string,
        secretHex: string
    ): Promise<string | null> {
        try {
            await db().query('insert into Device (id, name, secret_hex) values (?, ?, ?)', [
                id,
                name,
                secretHex,
            ]);
            return id;
        } catch (error) {
            console.log(error);
            return null;
        }
    }
    public static async revokeDevice(id: string): Promise<boolean> {
        const result = await db().query('update Device set revoked = 1 where id = ?', [id]);
        return result.affectedRows > 0;
    }
    public static async getDevices(): Promise<DeviceRow[]> {
        return await db().query('select id, name, updated_at from Device where revoked = 0', []);
    }
    public static async getById(id: string): Promise<DeviceRow | null> {
        const rows = await db().query(
            'select id, name, updated_at, revoked, secret_hex from Device where id = ?',
            [id]
        );
        if (rows.length) return rows[0];
        return null;
    }
}
