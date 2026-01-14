import { createFileRoute, useNavigate } from '@tanstack/react-router';
import { Layout, Button, Card, Form, Input, Space, Typography, type GetProps } from 'antd';
import { UserController, type LoginForm } from '../../data/user';
import { useUser } from 'src/providers/UserProvider';
import { useState } from 'react';

const { Title, Paragraph, Text, Link } = Typography;

export const Route = createFileRoute('/auth/login')({
    component: RouteComponent,
});

enum LOGIN_STEPS {
    PASSWORD,
    MFA_SETUP,
    MFA_VERIFY,
}

type OTPProps = GetProps<typeof Input.OTP>;

function RouteComponent() {
    const [step, setStep] = useState(LOGIN_STEPS.PASSWORD);
    const [qrCode, setQrCode] = useState<string>();
    const [error, setError] = useState<string>();

    const { setIsLoggedIn, setName } = useUser();
    const navigate = useNavigate();

    async function doLogin(data: LoginForm) {
        setError(undefined);

        const res = await UserController.login(data);
        if ('error' in res) {
            setError(res.error);
        } else if (res.mfaSetupRequired) {
            setQrCode(res.qrCode!);
            setStep(LOGIN_STEPS.MFA_SETUP);
        } else setStep(LOGIN_STEPS.MFA_VERIFY);
    }

    const doOtpVerify: OTPProps['onChange'] = async text => {
        const res = await UserController.verifyTotp(text);
        console.log(res);
        if ('error' in res) setError(res.error);
        else {
            // success!
            setIsLoggedIn(true);
            setName(res.name);
            navigate({
                to: '/dashboard',
            });
        }
    };

    return (
        <Layout.Content className='p-8  max-w-4xl mx-auto'>
            <Space direction='vertical' align='center'>
                <Title>CMB Dashboard Login</Title>

                <Card>
                    {step === LOGIN_STEPS.PASSWORD ? (
                        <Space direction='vertical' size={16}>
                            <Form
                                name='login'
                                initialValues={{ remember: true }}
                                onFinish={doLogin}
                            >
                                <Form.Item<LoginForm>
                                    label='Username'
                                    name='username'
                                    rules={[{ required: true }]}
                                >
                                    <Input autoComplete='current-username' />
                                </Form.Item>
                                <Form.Item<LoginForm>
                                    label='Password'
                                    name='password'
                                    rules={[{ required: true }]}
                                >
                                    <Input.Password autoComplete='current-password' />
                                </Form.Item>
                                <Button type='primary' htmlType='submit' block>
                                    Login
                                </Button>
                            </Form>
                            {error ? (
                                <p className='text-center w-full text-red-500 mb-4'>{error}</p>
                            ) : null}
                        </Space>
                    ) : step === LOGIN_STEPS.MFA_SETUP ? (
                        <Space direction='vertical' align='center' size={8}>
                            <Typography.Text>
                                Scan the qr code with your authenticator app to setup 2FA and then
                                press proceed.
                            </Typography.Text>
                            <img src={qrCode} />
                            <Button
                                type='primary'
                                block
                                onClick={() => setStep(LOGIN_STEPS.MFA_VERIFY)}
                            >
                                Proceed
                            </Button>
                        </Space>
                    ) : (
                        <Space direction='vertical' align='center' size={8}>
                            <Typography.Text>Enter 2FA Token</Typography.Text>
                            <Input.OTP onChange={doOtpVerify} />
                            <Button type='primary' block>
                                Submit
                            </Button>
                            {error ? (
                                <p className='text-center w-full text-red-500 mb-4'>{error}</p>
                            ) : null}
                        </Space>
                    )}
                </Card>
            </Space>
        </Layout.Content>
    );
}
