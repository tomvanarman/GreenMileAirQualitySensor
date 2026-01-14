# Network & SSL

SSL is enabled by default. Certificate are automatically renewed using Certbot.

## Setup

On the initial setup we need to request a certificate.

Replace the domain name and email with your own.

```bash
docker compose run --rm certbot certonly \
  --webroot \
  -w /var/www/certbot \
  -d example.com \
  --email you@example.com \
  --agree-tos \
  --no-eff-email
```

THen restart the server to load the new certificate.

```bash
docker compose exec nginx nginx -s reload
```
