import { createId } from '@paralleldrive/cuid2';
import { hashHex } from '../crypto';
import db from '../db';

export interface UserRow {
    id: string;
    name: string;
    password_hash: string;
    mfa_enabled: boolean;
    mfa_secret: string | null;
}

export default class UserModel {
    public static async createUser(name: string, password: string) {
        const id = createId();
        const hash = await hashHex(password);

        try {
            await db().query('insert into User (id, name, password_hash) values (?, ?, ?);', [
                id,
                name,
                hash,
            ]);
            return id;
        } catch (error) {
            console.log('Could not create user:', error);
            return null;
        }
    }
    public static async getUserByName(name: string): Promise<UserRow | null> {
        const rows = await db().query<UserRow[]>('select * from User where name = ?;', [name]);
        return rows.length > 0 ? rows[0] : null;
    }
    public static async getUserById(id: string): Promise<UserRow | null> {
        const rows = await db().query<UserRow[]>('select * from User where id = ?;', [id]);
        return rows.length > 0 ? rows[0] : null;
    }
    /**
     * Set MFA secret for user
     * @param id
     * @param secret
     */
    public static async setMfaSecret(id: string, secret: string) {
        await db().query('update User set mfa_secret = ? where id = ?;', [secret, id]);
    }
    public static async enableMfa(id: string) {
        await db().query('update User set mfa_enabled = 1 where id = ?;', [id]);
    }
}
