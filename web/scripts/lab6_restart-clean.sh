#!/bin/bash
# FridgeFusion Lab #6 - Clean Restart Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "WARNING: This will remove ALL data and volumes!"
read -p "Are you sure? (yes/no): " -r

if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
    echo "Cancelled."
    exit 1
fi

echo "Stopping all services and removing volumes..."
docker-compose down -v --remove-orphans

echo "Cleaning up..."
docker system prune -f

echo "Starting fresh..."
"$SCRIPT_DIR/lab6_start.sh"
