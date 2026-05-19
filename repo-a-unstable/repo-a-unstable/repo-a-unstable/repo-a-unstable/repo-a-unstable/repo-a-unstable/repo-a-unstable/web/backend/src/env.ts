import dotenv from 'dotenv';

export const IS_PRODUCTION = process.env.NODE_ENV === 'production';

// configure and load .env variables
dotenv.config({
    path: IS_PRODUCTION ? undefined : '../../.env',
    quiet: IS_PRODUCTION,
});
