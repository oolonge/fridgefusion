#!/usr/bin/env bash
# FridgeFusion Lab #6 - Load Balancing Test

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

ENDPOINT="http://localhost:9000/api/v2/health"
NUM_REQUESTS=50

echo "Sending $NUM_REQUESTS GET requests to $ENDPOINT"

for i in $(seq 1 $NUM_REQUESTS); do
    curl -s "$ENDPOINT" > /dev/null 2>&1
    echo -n "."
done

echo ""
echo ""
echo "Request distribution (Gateway Services):"

# Get container logs and count requests
GW1_COUNT=$(docker compose logs gateway-service-1 --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')
GW2_COUNT=$(docker compose logs gateway-service-2 --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')
GW3_COUNT=$(docker compose logs gateway-service-3 --tail=200 2>&1 | grep "GET /api/v2/health" | wc -l | tr -d ' ')

TOTAL=$((GW1_COUNT + GW2_COUNT + GW3_COUNT))

if [ $TOTAL -eq 0 ]; then
    echo "No requests found in gateway logs"
    exit 1
fi

GW1_PERCENT=$(echo "scale=1; ($GW1_COUNT*100)/$TOTAL" | bc)
GW2_PERCENT=$(echo "scale=1; ($GW2_COUNT*100)/$TOTAL" | bc)
GW3_PERCENT=$(echo "scale=1; ($GW3_COUNT*100)/$TOTAL" | bc)

echo "  Gateway-1: $GW1_COUNT requests ($GW1_PERCENT%)"
echo "  Gateway-2: $GW2_COUNT requests ($GW2_PERCENT%)"
echo "  Gateway-3: $GW3_COUNT requests ($GW3_PERCENT%)"
echo ""
echo "Expected: Gateway-1 ~50%, Gateway-2 ~25%, Gateway-3 ~25%"
