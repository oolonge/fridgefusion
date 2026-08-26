#!/bin/bash
# FridgeFusion Lab #6 - Startup Script (Microservices Architecture)

set -e

if ! docker info > /dev/null 2>&1; then
    echo "Docker is not running. Please start Docker first."
    exit 1
fi

echo "Building Docker images..."
docker-compose build

echo "Starting all services..."
docker-compose up -d

echo "Waiting for services to be healthy..."
sleep 15

echo "Service Status:"
docker-compose ps

echo ""
echo "FridgeFusion Lab #6 is running!"
echo "Architecture: Gateway -> Auth -> Core -> Data -> PostgreSQL (4 services x 3 instances)"
echo ""
echo "Endpoints:"
echo "  Main:     http://localhost:9000"
echo "  Mirror:   http://localhost:9000/mirror"
echo "  Health:   http://localhost:9000/health"
echo "  Grafana:  http://localhost:3000 (admin/admin)"
echo ""
echo "Commands:"
echo "  Stop:          ./scripts/lab6_stop.sh"
echo "  Clean restart: ./scripts/lab6_restart-clean.sh"
echo "  Logs:          docker-compose logs -f [service-name]"
