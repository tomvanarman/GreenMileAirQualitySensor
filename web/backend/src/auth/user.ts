import express from 'express';

/**
 * Logout user by destroying the session
 * @param req
 * @param res
 * @returns
 */
export async function logout(req: express.Request, res: express.Response): Promise<unknown> {
    return new Promise((resolve, rej) => {
        req.session.destroy(error => {
            // default cookie name
            console.log('clearing cookie');
            res.clearCookie('connect.sid');
            if (error) {
                rej(error);
            } else {
                resolve(null);
            }
        });
    });
}

/**
 * Check if user is authenticated (logged in and MFA verified)
 * @param req
 * @returns
 */
export function userIsAuthenticated(req: express.Request): boolean {
    return Boolean(req.session.isLoggedIn) && Boolean(req.session.mfaVerified);
}

/**
 * Express middleware to check if user is authenticated
 * Sends 401 Unauthorized if not authenticated
 * @param req
 * @param res
 * @param next
 * @returns
 */
export const authMiddleware = async (
    req: express.Request,
    res: express.Response,
    next: express.NextFunction
) => {
    if (!userIsAuthenticated(req)) {
        await logout(req, res);
        return res.status(401).json({ error: 'Unauthorized' });
    }
    next();
};
