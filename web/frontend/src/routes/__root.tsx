import { createRootRoute, createRootRouteWithContext, Link, Outlet } from '@tanstack/react-router';
import { TanStackRouterDevtools } from '@tanstack/react-router-devtools';
import { App, ConfigProvider, Layout, theme } from 'antd';
import { UserProvider } from 'src/providers/UserProvider';
import type { RouterContext } from 'src/util/router';

const RootLayout = () => (
    <ConfigProvider
        theme={{
            token: {
                colorPrimary: '#6a9d4d',
            },
            algorithm: theme.darkAlgorithm,
        }}
    >
        <App>
            <UserProvider>
                <Layout>
                    <Outlet />
                </Layout>
            </UserProvider>

            <TanStackRouterDevtools />
        </App>
    </ConfigProvider>
);

export const Route = createRootRouteWithContext<RouterContext>()({
    component: RootLayout,
});
