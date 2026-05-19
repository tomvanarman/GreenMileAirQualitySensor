import { IS_PRODUCTION } from './env';
import FileStoreFactory from 'session-file-store';
import session from 'express-session';

const FileStore = FileStoreFactory(session);

declare module 'express-session' {
    interface SessionData {
        isLoggedIn: boolean;
        userId: string;
        mfaVerified: boolean;
        name: string;
    }
}

export const sessionMiddleware = session({
    secret: process.env.CRYPTO_SECRET!,
    saveUninitialized: false,
    resave: false,
    cookie: {
        secure: IS_PRODUCTION,
    },
    store: new FileStore(),
});
