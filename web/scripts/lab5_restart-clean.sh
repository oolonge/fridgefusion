#!/bin/bash
# FridgeFusion Lab #5 - Clean Restart Script
# Stops all services and removes volumes for fresh start

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

COMPOSE_FILE="docker-compose.lab5.yml"

echo "========================================="
echo "FridgeFusion Lab #5 - Clean Restart"
echo "========================================="
echo ""

echo "WARNING: This will remove ALL data and volumes!"
echo ""
echo "This will:"
echo "  - Stop all running containers"
echo "  - Remove all volumes (database data, logs, etc.)"
echo "  - Start fresh with new configuration"
echo ""
read -p "Are you sure? (yes/no): " -r
echo ""

if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
    echo "Cancelled."
    exit 1
fi

echo "Stopping all services..."
docker compose -f "$COMPOSE_FILE" down -v

echo ""
echo "Cleaning up..."
docker system prune -f

echo ""
echo "Starting fresh..."
"$SCRIPT_DIR/lab5_start.sh"
