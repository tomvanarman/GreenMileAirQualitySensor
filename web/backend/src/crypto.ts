import argon2 from 'argon2';
import crypto from 'node:crypto';

const CRYPTO_SECRET = Buffer.from(process.env.CRYPTO_SECRET!, 'hex');

/**
 * Hash a value using Argon2 with the global CRYPTO_SECRET.
 * @param value
 * @returns
 */
export async function hashHex(value: string): Promise<string> {
    return argon2.hash(value, {
        secret: CRYPTO_SECRET,
    });
}

/**
 * Verify a value against an Argon2 hash using the global CRYPTO_SECRET.
 * @param value
 * @param hash
 * @returns
 */
export async function verifyHash(value: string, hash: string): Promise<boolean> {
    return argon2.verify(hash, value, {
        secret: CRYPTO_SECRET,
    });
}

/**
 * Derive a new secret key from the CRYPTO_SECRET using HKDF with SHA-256.
 * @param salt A unique salt value for the derivation.
 */
export function deriveKeyHex(salt: string): string {
    const buffer = crypto.hkdfSync(
        'sha256',
        CRYPTO_SECRET,
        Buffer.from(salt),
        Buffer.from('device-hmac-key'),
        32
    );
    return Buffer.from(buffer).toString('hex');
}

/**
 * Verify the HMAC (SHA256) signature of a device message body.
 * It is compatible with the Mbed TLS HMAC implementation.
 * @param signature given HMAC signature in hex
 * @param body the raw body buffer
 * @param secretHex secret key in hex
 * @returns
 */
export async function verifyDeviceHmac(signature: string, body: Buffer, secretHex: string) {
    try {
        const hmac = crypto.createHmac('sha256', Buffer.from(secretHex, 'hex'));
        hmac.update(body);
        const digest = hmac.digest('hex');
        return crypto.timingSafeEqual(Buffer.from(signature, 'hex'), Buffer.from(digest, 'hex'));
    } catch (error) {
        console.log('Error verifying device HMAC:', error);
        return false;
    }
}
