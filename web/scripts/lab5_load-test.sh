#!/usr/bin/env bash
# FridgeFusion Lab #5 - Load Testing Script
# Uses ApacheBench (ab) for performance testing

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"

COMPOSE_FILE="docker-compose.lab5.yml"

# Configuration
BASE_URL="http://localhost:9000"
REQUESTS=10000
CONCURRENCY=100
KEEP_ALIVE="-k"
TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
TIMESTAMP_FILE=$(date +%Y%m%d_%H%M%S)

echo "========================================="
echo "FridgeFusion Load Testing"
echo "========================================="
echo "Base URL: $BASE_URL"
echo "Timestamp: $TIMESTAMP"
echo ""

# Check if ApacheBench is available
if ! command -v ab &> /dev/null; then
    echo "Error: ApacheBench (ab) is not installed"
    echo "Install it with: brew install httpd"
    exit 1
fi

# Create results directory
mkdir -p load-test-results
RESULTS_DIR="load-test-results/$TIMESTAMP_FILE"
mkdir -p "$RESULTS_DIR"

echo "Starting load tests..."
echo ""

# Create JSON payloads for authentication tests
cat > "$RESULTS_DIR/login-success.json" << 'EOF'
{"username":"a","password":"a"}
EOF

cat > "$RESULTS_DIR/login-fail.json" << 'EOF'
{"username":"invalid","password":"invalid"}
EOF

# Test 1: Health Endpoint (GET request, will be load balanced 2:1:1)
echo "[1/4] Testing Health Check Endpoint (GET /api/v2/health)"
ab -n $REQUESTS -c $CONCURRENCY $KEEP_ALIVE \
  -g "$RESULTS_DIR/health-plot.tsv" \
  "$BASE_URL/api/v2/health" > "$RESULTS_DIR/health-results.txt" 2>&1
echo "Completed"
echo ""

# Test 2: Authentication - Successful login
echo "[2/4] Testing Authentication - Successful Login (POST /api/v2/auth/login)"
ab -n 1000 -c 50 -p "$RESULTS_DIR/login-success.json" -T "application/json" \
  "$BASE_URL/api/v2/auth/login" > "$RESULTS_DIR/auth-success-results.txt" 2>&1
echo "Completed"
echo ""

# Test 3: Authentication - Failed login
echo "[3/4] Testing Authentication - Failed Login (POST /api/v2/auth/login)"
ab -n 1000 -c 50 -p "$RESULTS_DIR/login-fail.json" -T "application/json" \
  "$BASE_URL/api/v2/auth/login" > "$RESULTS_DIR/auth-fail-results.txt" 2>&1
echo "Completed"
echo ""

# Test 4: Mirror Health Endpoint (GET request, will be load balanced 2:1:1)
echo "[4/4] Testing Mirror Health Check (GET /mirror/api/v2/health)"
ab -n $REQUESTS -c $CONCURRENCY $KEEP_ALIVE \
  -g "$RESULTS_DIR/mirror-health-plot.tsv" \
  "$BASE_URL/mirror/api/v2/health" > "$RESULTS_DIR/mirror-health-results.txt" 2>&1
echo "Completed"
echo ""

# Generate Markdown report
echo "Generating report..."

# Function to extract metrics from ab output
extract_value() {
    local file=$1
    local pattern=$2
    grep "$pattern" "$file" | head -1 | awk '{for(i=1;i<=NF;i++) if($i ~ /^[0-9]+(\.[0-9]+)?$/) {print $i; exit}}'
}

extract_percentile() {
    local file=$1
    local percentile=$2
    grep "^ *${percentile}%" "$file" | awk '{print $2}'
}

# Extract metrics for all tests
RPS_HEALTH=$(extract_value "$RESULTS_DIR/health-results.txt" "Requests per second:")
TIME_HEALTH=$(extract_value "$RESULTS_DIR/health-results.txt" "Time per request:")
P50_HEALTH=$(extract_percentile "$RESULTS_DIR/health-results.txt" "50")
P90_HEALTH=$(extract_percentile "$RESULTS_DIR/health-results.txt" "90")
P95_HEALTH=$(extract_percentile "$RESULTS_DIR/health-results.txt" "95")
P99_HEALTH=$(extract_percentile "$RESULTS_DIR/health-results.txt" "99")
FAILED_HEALTH=$(grep "Failed requests:" "$RESULTS_DIR/health-results.txt" | awk '{print $3}')

RPS_AUTH_SUCCESS=$(extract_value "$RESULTS_DIR/auth-success-results.txt" "Requests per second:")
TIME_AUTH_SUCCESS=$(extract_value "$RESULTS_DIR/auth-success-results.txt" "Time per request:")
P50_AUTH_SUCCESS=$(extract_percentile "$RESULTS_DIR/auth-success-results.txt" "50")
P99_AUTH_SUCCESS=$(extract_percentile "$RESULTS_DIR/auth-success-results.txt" "99")
FAILED_AUTH_SUCCESS=$(grep "Failed requests:" "$RESULTS_DIR/auth-success-results.txt" | awk '{print $3}')

RPS_AUTH_FAIL=$(extract_value "$RESULTS_DIR/auth-fail-results.txt" "Requests per second:")
TIME_AUTH_FAIL=$(extract_value "$RESULTS_DIR/auth-fail-results.txt" "Time per request:")
P50_AUTH_FAIL=$(extract_percentile "$RESULTS_DIR/auth-fail-results.txt" "50")
P99_AUTH_FAIL=$(extract_percentile "$RESULTS_DIR/auth-fail-results.txt" "99")
FAILED_AUTH_FAIL=$(grep "Failed requests:" "$RESULTS_DIR/auth-fail-results.txt" | awk '{print $3}')

RPS_MIRROR=$(extract_value "$RESULTS_DIR/mirror-health-results.txt" "Requests per second:")
TIME_MIRROR=$(extract_value "$RESULTS_DIR/mirror-health-results.txt" "Time per request:")
P50_MIRROR=$(extract_percentile "$RESULTS_DIR/mirror-health-results.txt" "50")
P90_MIRROR=$(extract_percentile "$RESULTS_DIR/mirror-health-results.txt" "90")
P95_MIRROR=$(extract_percentile "$RESULTS_DIR/mirror-health-results.txt" "95")
P99_MIRROR=$(extract_percentile "$RESULTS_DIR/mirror-health-results.txt" "99")
FAILED_MIRROR=$(grep "Failed requests:" "$RESULTS_DIR/mirror-health-results.txt" | awk '{print $3}')

# Generate Markdown report
cat > "LOAD-TEST-REPORT.md" << EOF
# FridgeFusion Lab #5 - Load Test Report

**Date:** $TIMESTAMP
**Tool:** ApacheBench (ab)
**System:** macOS

---

## Test Configuration

| Test | Endpoint | Method | Requests | Concurrency |
|------|----------|--------|----------|-------------|
| Test 1 | /api/v2/health | GET | 10000 | 100 |
| Test 2 | /api/v2/auth/login | POST | 1000 | 50 |
| Test 3 | /api/v2/auth/login | POST | 1000 | 50 |
| Test 4 | /mirror/api/v2/health | GET | 10000 | 100 |

---

## Results

### Test 1: Health Check Endpoint (GET)

| Metric | Value |
|--------|-------|
| Requests per second | ${RPS_HEALTH} req/sec |
| Time per request (mean) | ${TIME_HEALTH} ms |
| p50 | ${P50_HEALTH} ms |
| p90 | ${P90_HEALTH} ms |
| p95 | ${P95_HEALTH} ms |
| p99 | ${P99_HEALTH} ms |
| Failed requests | ${FAILED_HEALTH} |

### Test 2: Authentication - Successful Login (POST)

| Metric | Value |
|--------|-------|
| Requests per second | ${RPS_AUTH_SUCCESS} req/sec |
| Time per request (mean) | ${TIME_AUTH_SUCCESS} ms |
| p50 | ${P50_AUTH_SUCCESS} ms |
| p99 | ${P99_AUTH_SUCCESS} ms |
| Failed requests | ${FAILED_AUTH_SUCCESS} |

**Payload:** \`{"username":"a","password":"a"}\`

### Test 3: Authentication - Failed Login (POST)

| Metric | Value |
|--------|-------|
| Requests per second | ${RPS_AUTH_FAIL} req/sec |
| Time per request (mean) | ${TIME_AUTH_FAIL} ms |
| p50 | ${P50_AUTH_FAIL} ms |
| p99 | ${P99_AUTH_FAIL} ms |
| Failed requests | ${FAILED_AUTH_FAIL} |

**Payload:** \`{"username":"invalid","password":"invalid"}\`

### Test 4: Mirror Health Check (GET)

| Metric | Value |
|--------|-------|
| Requests per second | ${RPS_MIRROR} req/sec |
| Time per request (mean) | ${TIME_MIRROR} ms |
| p50 | ${P50_MIRROR} ms |
| p90 | ${P90_MIRROR} ms |
| p95 | ${P95_MIRROR} ms |
| p99 | ${P99_MIRROR} ms |
| Failed requests | ${FAILED_MIRROR} |

---

## Summary Table

| Test | Endpoint | RPS | Mean (ms) | p50 (ms) | p99 (ms) | Failed |
|------|----------|-----|-----------|----------|----------|--------|
| 1 | /api/v2/health | ${RPS_HEALTH} | ${TIME_HEALTH} | ${P50_HEALTH} | ${P99_HEALTH} | ${FAILED_HEALTH} |
| 2 | /auth/login (success) | ${RPS_AUTH_SUCCESS} | ${TIME_AUTH_SUCCESS} | ${P50_AUTH_SUCCESS} | ${P99_AUTH_SUCCESS} | ${FAILED_AUTH_SUCCESS} |
| 3 | /auth/login (fail) | ${RPS_AUTH_FAIL} | ${TIME_AUTH_FAIL} | ${P50_AUTH_FAIL} | ${P99_AUTH_FAIL} | ${FAILED_AUTH_FAIL} |
| 4 | /mirror/api/v2/health | ${RPS_MIRROR} | ${TIME_MIRROR} | ${P50_MIRROR} | ${P99_MIRROR} | ${FAILED_MIRROR} |

---

## Load Balancing

GET requests (Test 1 and Test 4) are distributed in 2:1:1 ratio across backend instances.
POST requests (Test 2 and Test 3) are routed only to master instance.

---

## Detailed Results

Full ApacheBench reports saved in: \`$RESULTS_DIR/\`

- \`health-results.txt\`
- \`auth-success-results.txt\`
- \`auth-fail-results.txt\`
- \`mirror-health-results.txt\`

EOF

echo "========================================="
echo "Load testing completed"
echo "========================================="
echo ""
echo "Report generated: LOAD-TEST-REPORT.md"
echo "Raw results: $RESULTS_DIR/"
echo ""
echo "To check load balancing distribution:"
echo "  docker compose -f $COMPOSE_FILE logs caddy --tail=1000 | grep 'GET /api/v2/health'"
echo ""
