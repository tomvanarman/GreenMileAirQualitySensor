import { makeFetch } from 'src/util/fetch';

export type LoginForm = {
    username: string;
    password: string;
};

type ErrorRespose = {
    error: string;
};

export type LoginResponse =
    | ErrorRespose
    | {
          success: true;
          mfaSetupRequired: boolean;
          qrCode?: string;
      };

export type ProfileResponse = ErrorRespose | { name: string };

export type TotpVerifyResponse = ErrorRespose | { success: true; name: string };

export const UserController = {
    async login({ username, password }: LoginForm): Promise<LoginResponse> {
        const response = await makeFetch('/api/user/login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ username, password }),
            credentials: 'include',
        });
        return response.json();
    },
    async verifyTotp(token: string): Promise<TotpVerifyResponse> {
        const response = await makeFetch('/api/user/login/mfa/verify', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ token }),
            credentials: 'include',
        });
        return response.json();
    },
    async getProfile(): Promise<ProfileResponse> {
        const response = await makeFetch('/api/user/profile');
        return response.json();
    },
};
