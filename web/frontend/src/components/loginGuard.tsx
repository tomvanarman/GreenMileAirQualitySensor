import { useNavigate } from '@tanstack/react-router';
import { useEffect, useState } from 'react';
import { UserController } from 'src/data/user';
import { useUser } from 'src/providers/UserProvider';

/**
 * Higher Order Component that checks if a user is logged in. If so it fetches the profile and else it redirects to the public page
 * @param Component
 * @returns
 */
export function LoginGuard(Component: React.ComponentType<any>) {
    return (props: any) => {
        const [isLoading, setIsLoading] = useState(true);
        const { setIsLoggedIn, setName } = useUser();

        const navigate = useNavigate();

        useEffect(() => {
            UserController.getProfile()
                .then(async res => {
                    if ('error' in res) {
                        setIsLoggedIn(false);
                        setIsLoading(false);
                        navigate({ to: '..' });
                    } else {
                        setIsLoading(false);
                        setIsLoggedIn(true);
                        setName(res.name);
                    }
                })
                .catch(() => {
                    setIsLoggedIn(false);
                    setIsLoading(false);
                    navigate({ to: '..' });
                });
        }, []);

        return isLoading ? <p>Loading...</p> : <Component {...props} />;
    };
}
