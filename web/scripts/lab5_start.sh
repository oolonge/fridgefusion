#!/bin/bash
# FridgeFusion Lab #5 - Startup Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

COMPOSE_FILE="docker-compose.lab5.yml"

echo "========================================="
echo "FridgeFusion Lab #5 - Starting Services"
echo "========================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if Docker is running
if ! docker info > /dev/null 2>&1; then
    echo "Docker is not running. Please start Docker first."
    exit 1
fi

echo "Docker is running"
echo ""

# Copy Lab5 Caddyfile
echo "Setting up Lab #5 Caddyfile..."
cp docker/caddy/Caddyfile.lab5 docker/caddy/Caddyfile

# Build and start services
echo "Building Docker images (this may take a while on first run)..."
docker compose -f "$COMPOSE_FILE" build

echo ""
echo "Starting all services..."
docker compose -f "$COMPOSE_FILE" up -d

echo ""
echo "Waiting for services to be healthy..."
sleep 10

# Check service status
echo ""
echo "Service Status:"
docker compose -f "$COMPOSE_FILE" ps

echo ""
echo "${GREEN}=========================================${NC}"
echo "${GREEN}FridgeFusion Lab #5 is running!${NC}"
echo "${GREEN}=========================================${NC}"
echo ""
echo "Available endpoints:"
echo ""
echo "  Main Application (Lab #5):"
echo "     - HTTP:  http://localhost:9000"
echo "     - HTTPS: https://localhost:9443"
echo ""
echo "  Mirror Application (Lab #5):"
echo "     - HTTP:  http://localhost:9000/mirror"
echo "     - HTTPS: https://localhost:9443/mirror"
echo ""
echo "  Monitoring:"
echo "     - Grafana: http://localhost:3000 (admin/admin)"
echo "     - Loki:    http://localhost:3100"
echo ""
echo "  Direct Backend Access:"
echo "     - Master:  http://localhost:8080/api/v2"
echo "     - RO1:     http://localhost:8081/api/v2"
echo "     - RO2:     http://localhost:8082/api/v2"
echo ""
echo "  Database:"
echo "     - Master:  localhost:5434"
echo "     - Slave:   localhost:5435"
echo ""
echo "${YELLOW}Note: Accept the self-signed certificate in your browser${NC}"
echo ""
echo "To view logs: docker compose -f $COMPOSE_FILE logs -f [service-name]"
echo "To stop:      ./scripts/lab5_stop.sh"
echo ""
