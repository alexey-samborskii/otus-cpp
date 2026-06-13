#!/usr/bin/env bash

set -euo pipefail

project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cert_dir="${project_dir}/certs"

mkdir -p "${cert_dir}"

openssl req \
    -x509 \
    -newkey rsa:2048 \
    -sha256 \
    -nodes \
    -days 365 \
    -keyout "${cert_dir}/server.key" \
    -out "${cert_dir}/server.crt" \
    -subj "/CN=localhost" \
    -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"

chmod 600 "${cert_dir}/server.key"

echo "Certificate: ${cert_dir}/server.crt"
echo "Private key: ${cert_dir}/server.key"
