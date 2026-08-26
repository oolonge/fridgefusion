#!/bin/bash
# FridgeFusion Lab #5 - Check Replication Status

set -e

echo "PostgreSQL Replication Status"
echo ""

echo "Master replication connections:"
docker exec fridgefusion-postgres-master psql -U postgres -d fridgefusion -c "SELECT application_name, state, sync_state FROM pg_stat_replication;" 2>/dev/null || echo "Master not available"

echo ""
echo "Slave recovery status:"
docker exec fridgefusion-postgres-slave psql -U postgres -d fridgefusion -c "SELECT pg_is_in_recovery() as is_replica;" 2>/dev/null || echo "Slave not available"

echo ""
echo "Replication lag:"
docker exec fridgefusion-postgres-slave psql -U postgres -d fridgefusion -c "SELECT CASE WHEN pg_last_wal_receive_lsn() = pg_last_wal_replay_lsn() THEN 0 ELSE EXTRACT(EPOCH FROM now() - pg_last_xact_replay_timestamp()) END AS lag_seconds;" 2>/dev/null || echo "Could not check lag"
