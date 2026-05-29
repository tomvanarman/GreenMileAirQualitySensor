import { createContext, useContext, useState, type PropsWithChildren } from 'react';

type UserContextType = {
    name: string;
    setName: React.Dispatch<React.SetStateAction<string>>;
    isLoggedIn: boolean;
    setIsLoggedIn: React.Dispatch<React.SetStateAction<boolean>>;
};

const UserContext = createContext<UserContextType | null>(null);

export function UserProvider(props: PropsWithChildren) {
    const [name, setName] = useState<string>('');
    const [isLoggedIn, setIsLoggedIn] = useState(false);

    return (
        <UserContext.Provider value={{ name, setName, isLoggedIn, setIsLoggedIn }}>
            {props.children}
        </UserContext.Provider>
    );
}

export function useUser(): UserContextType {
    const ctx = useContext(UserContext);
    if (!ctx) throw new Error('useUser must be used within UserProvider');

    return ctx;
}
