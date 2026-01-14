import { createFileRoute, Outlet, useParams } from '@tanstack/react-router';
import { Layout, Typography, Input, Space, Button } from 'antd';
import { Chart } from '../components/chart';
import { QualityController } from '../data/quality';
import { useEffect, useState } from 'react';

export const Route = createFileRoute('/')({
    component: RouteComponent,
});

async function fetchData(deviceId: string) {
    const response = await QualityController.getQualityData(deviceId);
    if ('error' in response) return null;
    return response.quality;
}

function RouteComponent() {
    const search = Route.useSearch();

    const [percent, SetPercent] = useState(0);
    const [isLoading, setIsLoading] = useState(true);
    const [error, setError] = useState<string>();

    useEffect(() => {
        // fetch data
        fetchData((search as any).deviceId ?? '').then(p => {
            if (p) {
                SetPercent(p * 100);
                setIsLoading(false);
            } else {
                setError('No measurements yet');
            }
        });
    }, []);

    return (
        <Layout.Content className='p-8 max-w-4xl mx-auto'>
            <Typography.Title className='text-center'>
                How good is the air around you?
            </Typography.Title>
            <p className='text-xl w-full text-center'>
                This project is part of{' '}
                <a href='https://www.greenmileamsterdam.com/' target='_blank'>
                    <span
                        className='font-bold'
                        style={{ color: '#058b42', textDecoration: 'underline' }}
                    >
                        The Green Mile
                    </span>
                </a>
            </p>
            <Chart percent={percent} isLoading={isLoading} />
            {error ? (
                <p className='text-center w-full text-red-500 mb-4'>
                    No measurements yet. Please try again later
                </p>
            ) : null}
            <Space direction='vertical' size={16}>
                <Typography.Title level={3}>What do these numbers mean?</Typography.Title>
                <Typography.Text>
                    Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor
                    incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis
                    nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
                    Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu
                    fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in
                    culpa qui officia deserunt mollit anim id est laborum
                </Typography.Text>
            </Space>
        </Layout.Content>
    );
}
