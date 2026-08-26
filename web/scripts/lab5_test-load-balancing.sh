#!/usr/bin/env bash
# FridgeFusion Lab #5 - Load Balancing Test

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

COMPOSE_FILE="docker-compose.lab5.yml"
ENDPOINT="http://localhost:9000/api/v2/health"
NUM_REQUESTS=50

echo "Sending $NUM_REQUESTS GET requests to $ENDPOINT"

for i in $(seq 1 $NUM_REQUESTS); do
    curl -s "$ENDPOINT" > /dev/null 2>&1
    echo -n "."
done

echo ""
echo ""
echo "Request distribution:"

# Get container logs and count requests
MASTER_COUNT=$(docker compose -f "$COMPOSE_FILE" logs backend-master --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')
RO1_COUNT=$(docker compose -f "$COMPOSE_FILE" logs backend-ro1 --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')
RO2_COUNT=$(docker compose -f "$COMPOSE_FILE" logs backend-ro2 --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')

TOTAL=$((MASTER_COUNT + RO1_COUNT + RO2_COUNT))

if [ $TOTAL -eq 0 ]; then
    echo "No requests found in backend logs"
    exit 1
fi

MASTER_PERCENT=$(echo "scale=1; ($MASTER_COUNT*100)/$TOTAL" | bc)
RO1_PERCENT=$(echo "scale=1; ($RO1_COUNT*100)/$TOTAL" | bc)
RO2_PERCENT=$(echo "scale=1; ($RO2_COUNT*100)/$TOTAL" | bc)

echo "  Master: $MASTER_COUNT requests ($MASTER_PERCENT%)"
echo "  RO1:    $RO1_COUNT requests ($RO1_PERCENT%)"
echo "  RO2:    $RO2_COUNT requests ($RO2_PERCENT%)"
echo ""
echo "Expected: Master ~50%, RO1 ~25%, RO2 ~25%"
