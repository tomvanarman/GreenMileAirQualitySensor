import express from 'express';
import UserModel from './user';
import { verifyHash } from '../crypto';
import { authMiddleware, logout } from '../auth/user';
import speakeasy from 'speakeasy';
import QRCode from 'qrcode';
import { sessionMiddleware } from '../session';

export const userRouter = express.Router();
userRouter.use(sessionMiddleware);

/**
 * Login with username and password and MFA:
 * 1. User logins with username and password
 * 2. If user has MFA setup, indicate that we need to get a MFA token
 * 3. If user does not have MFA enabled, indicate that we need to setup MFA
 */
userRouter.post('/login', async (req, res) => {
    // verify user input
    const { username, password } = req.body;

    if (!username || !password) {
        return res.status(400).json({ error: 'Username and password are required' });
    }

    const user = await UserModel.getUserByName(username);
    if (!user) {
        return res.status(401).json({ error: 'Invalid username or password' });
    }

    // verify password
    const isValidPassword = await verifyHash(password, user.password_hash);
    if (!isValidPassword) {
        return res.status(401).json({ error: 'Invalid username or password' });
    }

    // setup session
    req.session.userId = user.id;
    req.session.name = user.name;
    req.session.isLoggedIn = true;
    req.session.mfaVerified = false;
    req.session.save();

    if (user.mfa_enabled) {
        return res.json({
            success: true,
            mfaSetupRequired: false,
        });
    }
    // MFA needs to be setup

    // generate MFA secret
    const secret = speakeasy.generateSecret({
        length: 32,
        name: `CMB (${user.name})`,
        issuer: 'CMB',
    });
    // store MFA secret for user
    await UserModel.setMfaSecret(user.id, secret.base32);

    // generate QR code as data URL
    const qrUrl = await QRCode.toDataURL(secret.otpauth_url!);

    res.json({
        qrCode: qrUrl,
        success: true,
        mfaSetupRequired: true,
    });
});

userRouter.post('/login/mfa/verify', async (req, res) => {
    if (!req.session.isLoggedIn) {
        await logout(req, res);
        return res.status(401).json({ error: 'Not logged in' });
    } else if (req.session.mfaVerified) {
        return res.status(400).json({ error: 'MFA already verified' });
    }

    // verify user input
    const { token } = req.body;
    if (!token) {
        return res.status(400).json({ error: 'MFA token is required' });
    }
    // check if user has MFA enabled
    const user = await UserModel.getUserById(req.session.userId!);
    if (!user || !user.mfa_secret) {
        await logout(req, res);
        return res.status(401).json({ error: 'Not logged in' });
    }

    // verify MFA
    const verified = speakeasy.totp.verify({
        secret: user.mfa_secret,
        encoding: 'base32',
        token,
        window: 2, // 60 seconds
    });

    if (verified) {
        if (!user.mfa_enabled) {
            await UserModel.enableMfa(user.id);
        }

        req.session.mfaVerified = true;
        req.session.save();
        res.json({ success: true, name: user.name });
    } else {
        res.status(401).json({ error: 'Invalid MFA token' });
    }
});

// All routes below require authentication
// ================================
userRouter.use(authMiddleware);

userRouter.post('/logout', async (req, res) => {
    await logout(req, res);
    res.json({ success: true });
});

userRouter.get('/profile', async (req, res) => {
    const user = await UserModel.getUserById(req.session.userId!);
    if (!user) {
        await logout(req, res);
        return res.status(401).json({
            error: 'Unauthorized',
        });
    }

    req.session.name = user.name;
    req.session.save();

    res.json({
        name: req.session.name,
    });
});
