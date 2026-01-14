# Network & SSL

We cannot open any other port except 443 due to firewall constraints. We use
[Traeffik](https://traefik.io/traefik) as reverse proxy to host multiple services on the same server
over port 443.

## Why Traeffik?

We chose to use Traeffik for the following features:

- Automatically renewing SSL certificates using Lets Encrypt
- The ability to serve multiple SSL applications over a single port
- It is possible to proxy not only HTTP traffic, but also pass through TLS traffic

## DNS

Each service is served on a different subdomain. At this moment we use a temporary demain

- `cmb.yourdomain.com` serves the backend API / possible future user application
- `grafana.yourdomain.com` serves the Grafana frontend

## How?

Bascially, Traeffik inspects the TLS SNI (Server Name Identification) value during the TLS handshake
to determine where traffic should go. The TLS side is handled by Traeffik and internally proxies the
TCP traffic to the appropriate services.

Because Traeffik serves as the only outside connection we can make our docker services insecure. So
the NodeJS backend receives HTTP. And traefik receives TLS/MQTTS and handles that itself.

## Future setup

For our MVP this is not implemented, but for an actual product we advise that a local CA is created.
From that CA server and client certificates can be generated and used to authenticated the devices.

### Creating a Local CA

1. Go to certs directory

```bash
cd docker/data/certs
mkdir certificates
```

2. Generate local CA that is valid for 10 years.

```bash
openssl genrsa -out ca.key 4096
openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 -out ca.crt -subj "/CN=GreenMile CMB CA"
```

### Creating the Server certificate

3. Generate the certificate key and CSR (certificate signing request) for the server. Replace the
   domain with the domain that will be used. Right now its the temporary domain `cmb.yourdomain.com`

```bash
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr -subj "/CN=cmb.yourdomain.com"
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 825 -sha256
```

4. Create a SAN (Subject Alternative Name) file, which is used to validate the hostname. Again,
   update the domain name if it changes.

```
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names

[alt_names]
DNS.1 = cmb.yourdomain.com
```

Add `DNS.2 = localhost` for local testing.

5. Sign the CSR using our local CA

```bash
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 825 -sha256 -extfile server.ext
```

We now have a locally signed server certificate and key.

### Generating Client certificates

Next, we need to generate client certificates. These certificates are used to connect to the server
i.e. from the embedded device(s). Note that we need to generate a certificate for each client.

We created a bash script that you can use to generate a client, you only need to input the client
name.

```bash
cd docker/data/certs
bash scripts/generate-client.sh
```

The generated certificates will be outputted to `certs/certificates/clients/[client name]/`
