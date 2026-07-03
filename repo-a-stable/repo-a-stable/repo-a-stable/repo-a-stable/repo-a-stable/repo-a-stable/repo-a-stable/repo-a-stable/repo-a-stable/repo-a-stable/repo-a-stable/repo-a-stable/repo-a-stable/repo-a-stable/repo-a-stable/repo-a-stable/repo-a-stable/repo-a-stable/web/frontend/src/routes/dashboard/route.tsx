import { createFileRoute, Link } from '@tanstack/react-router';
import {
    Button,
    Card,
    Input,
    Layout,
    Popconfirm,
    Space,
    Table,
    Typography,
    type TableProps,
} from 'antd';
import { useEffect, useState } from 'react';
import { LoginGuard } from 'src/components/loginGuard';
import { DeviceController, type DeviceRow } from 'src/data/device';
import { useUser } from 'src/providers/UserProvider';

export const Route = createFileRoute('/dashboard')({
    component: LoginGuard(RouteComponent),
});

const dateFormatter = new Intl.DateTimeFormat('nl-NL', {
    dateStyle: 'medium',
    timeStyle: 'short',
});

async function downloadQR(deviceId: string) {
    const response = await DeviceController.getQRCode(deviceId);
    const blob = await response.blob();
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `device_${deviceId}_qrcode.png`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    window.URL.revokeObjectURL(url);
}

function RouteComponent() {
    const [loading, setLoading] = useState(false);
    const [devices, setDevices] = useState<DeviceRow[]>([]);
    const [name, setName] = useState<string>('');
    const [secret, setSecret] = useState<string>('');
    const [newId, setNewId] = useState<string>('');
    const { name: userName } = useUser();

    useEffect(() => {
        DeviceController.getDevices().then(response => {
            setDevices(response.devices);
            setLoading(false);
        });
    }, []);

    const registerDevice = async () => {
        if (!name) return;
        const result = await DeviceController.registerDevice(name);
        setSecret(result.secret);
        setNewId(result.id);
        setDevices([...devices, result]);
    };

    const deviceCols: TableProps<DeviceRow>['columns'] = [
        {
            title: 'Device Name',
            dataIndex: 'name',
            key: 'name',
        },
        {
            title: 'Device ID',
            dataIndex: 'id',
            key: 'idd',
        },
        {
            title: 'Last Updated',
            dataIndex: 'updated_at',
            key: 'updated_at',
            render: (_, device) => dateFormatter.format(new Date(device.updated_at)),
        },
        {
            title: 'QR Code',
            key: 'qrcode',
            render: (_, device) => (
                <Button type='primary' onClick={() => downloadQR(device.id)}>
                    Get QR-code
                </Button>
            ),
        },
        {
            title: 'View Data',
            key: 'view',
            render: (_, device) => (
                <Link to={'..'} search={{ deviceId: device.id }}>
                    View Data
                </Link>
            ),
        },
        {
            title: 'Revoke',
            key: 'revoke',
            render: (_, device) => (
                <Popconfirm
                    title={`Are you sure you want to revoke device "${device.name}"?`}
                    onConfirm={async () => {
                        await DeviceController.revokeDevice(device.id);
                        setDevices(devices.filter(d => d.id !== device.id));
                    }}
                    okText='Yes'
                    cancelText='No'
                >
                    <Button danger>Revoke</Button>
                </Popconfirm>
            ),
        },
    ];

    return loading ? (
        <p>...</p>
    ) : (
        <Layout.Content className='p-8 w-full max-w-4xl mx-auto'>
            <Space direction='vertical' className='w-full'>
                <Typography.Title level={2}>CMB Dashboard</Typography.Title>
                <Typography.Text>
                    Hello {userName}, manage your registered devices below. <br />
                    <b>Note:</b> Revoked devices will no longer be able to send data and are not
                    visible in the dashboard.
                </Typography.Text>
                <Table columns={deviceCols} dataSource={devices} />
                <Card>
                    <Space direction='vertical' className='w-full'>
                        <Typography.Title level={3}>Register new device</Typography.Title>
                        <Typography.Text>
                            When registering a new device you can obtain a secret key. The key along
                            with the device's ID need to be set on the device using the credential
                            manager. Otherwise the device won't be allowed to send data.
                        </Typography.Text>
                        <label htmlFor='deviceName'>Device Name</label>
                        <Input id='deviceName' onChange={e => setName(e.target.value)} />
                        <Button type='primary' disabled={!name} onClick={registerDevice}>
                            Register
                        </Button>
                        {secret && (
                            <Card type='inner' title='Device Registered Successfully'>
                                <p>
                                    Please save the following secret key. It will not be shown
                                    again!
                                </p>
                                <p>
                                    <b>Secret key (hex):</b>
                                </p>
                                <Typography.Text code>{secret}</Typography.Text>
                                <p>
                                    <b>Device id:</b>
                                </p>
                                <Typography.Text code>{newId}</Typography.Text>
                            </Card>
                        )}
                    </Space>
                </Card>
            </Space>
        </Layout.Content>
    );
}
