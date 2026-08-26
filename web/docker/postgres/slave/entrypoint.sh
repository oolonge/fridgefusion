#!/bin/bash
set -e

echo "========================================="
echo "PostgreSQL Slave Entrypoint Wrapper"
echo "========================================="

# Wait for master to be ready
echo "Waiting for master database to be ready..."
until PGPASSWORD=$POSTGRES_PASSWORD psql -h "$POSTGRES_MASTER_HOST" -p "$POSTGRES_MASTER_PORT" -U "$POSTGRES_USER" -d postgres -c 'SELECT 1' >/dev/null 2>&1; do
  echo "  Master not ready, retrying in 3 seconds..."
  sleep 3
done

echo "✓ Master is ready!"
echo ""

# Check if replication is already set up
if [ -s "$PGDATA/PG_VERSION" ] && [ -f "$PGDATA/standby.signal" ]; then
  echo "Replication already configured."
  echo "Starting PostgreSQL in hot standby mode..."
  echo "========================================="
  # Execute original docker-entrypoint.sh with postgres user
  exec gosu postgres docker-entrypoint.sh postgres
fi

# First-time setup: clone from master
echo "First-time setup: Cloning from master..."
echo ""

# Ensure PGDATA directory exists and is empty
mkdir -p "$PGDATA"
rm -rf "$PGDATA"/*

# Run pg_basebackup to clone from master
echo "Running pg_basebackup..."
PGPASSWORD=replicator_secret pg_basebackup \
  -h "$POSTGRES_MASTER_HOST" \
  -p "$POSTGRES_MASTER_PORT" \
  -D "$PGDATA" \
  -U replicator \
  -Fp \
  -Xs \
  -P \
  -R

echo ""
echo "✓ Base backup completed!"
echo ""

# Ensure standby.signal exists
touch "$PGDATA/standby.signal"

# Set correct ownership
chown -R postgres:postgres "$PGDATA"
chmod 700 "$PGDATA"

echo "✓ Replication setup complete!"
echo "Starting PostgreSQL in hot standby mode..."
echo "========================================="
echo ""

# Start PostgreSQL as postgres user
exec gosu postgres postgres
