import { createFileRoute } from '@tanstack/react-router';
import { Layout, Typography, Space } from 'antd';
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
        // fetch data and update state
        const update = async () => {
            const p = await fetchData((search as any).deviceId ?? '');
            if (p) {
                SetPercent(p * 100);
                setIsLoading(false);
                setError(undefined);
            } else {
                setError('No measurements yet');
            }
        };
        update();
        const timer = window.setInterval(update, 2000);
        return () => window.clearInterval(timer);
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
                <Typography.Title level={3}>What does it actually measure?</Typography.Title>
                <Typography.Text>
                    The air quality is calculated by measuring the particulate matter (fine
                    particles, fijnstof) in the air using a SPS30 sensor. <br />
                    The most damaging particulate matter has a diameter of less than 2.5
                    micrometers, also known as &quot;PM2.5&quot;. <br />
                    PM2.5 particles are more likely to travel into and deposit on the surface of the
                    deeper parts of the lung causing damage to your lungs. Read more about the
                    impact of particulate matter{' '}
                    <a
                        href='https://ww2.arb.ca.gov/resources/inhalable-particulate-matter-and-health'
                        target='_blank'
                    >
                        here
                    </a>
                    . <br /> <br />
                    The graph above shows the air pollution as a value from 0-100% where 0% means no
                    PM2.5 is detected, and 100% means the sensor measured its maximum value. <br />
                    <b>Note:</b> this is not a complete representation of the air quality. There are
                    many more factors, PM2.5 is just one of them.
                </Typography.Text>
            </Space>
        </Layout.Content>
    );
}
