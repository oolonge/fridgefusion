#!/bin/bash
# FridgeFusion Lab #5 - Stop Script

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

COMPOSE_FILE="docker-compose.lab5.yml"

echo "Stopping FridgeFusion Lab #5 services..."
docker compose -f "$COMPOSE_FILE" down

echo "All services stopped"
echo ""
echo "To remove volumes as well: docker compose -f $COMPOSE_FILE down -v"
