#!/bin/bash
set -e

echo "========================================="
echo "Initializing PostgreSQL Master..."
echo "========================================="

# Copy custom pg_hba.conf to PGDATA
echo "Applying custom pg_hba.conf..."
if [ -f /etc/postgresql/pg_hba.conf ]; then
    cp /etc/postgresql/pg_hba.conf "$PGDATA/pg_hba.conf"
    chmod 600 "$PGDATA/pg_hba.conf"
    chown postgres:postgres "$PGDATA/pg_hba.conf"
    echo "✓ pg_hba.conf applied"

    # Reload configuration
    echo "Reloading PostgreSQL configuration..."
    psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" -c "SELECT pg_reload_conf();"
fi
echo ""

# Create replication user
echo "Creating replication user..."
psql -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" --dbname "$POSTGRES_DB" <<-EOSQL
    -- Create replication user
    CREATE USER replicator WITH REPLICATION ENCRYPTED PASSWORD 'replicator_secret';

    -- Create read-only user for replica instances
    CREATE USER readonly_user WITH ENCRYPTED PASSWORD 'readonly_secret';
EOSQL

echo "Replication user created successfully!"
echo ""

# Note: Replication slot will be created by slave during pg_basebackup -C flag
echo "Replication slot will be created by slave during initial connection"
echo ""

# Note: init-db.sql is run automatically by postgres entrypoint (02-init-db.sql)
# It creates the schema and inserts test data
echo "Database schema will be created from 02-init-db.sql"
echo ""
echo "PostgreSQL Master initialization completed!"
echo "========================================="
