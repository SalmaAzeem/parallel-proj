#!/bin/bash

SERVICE_NAME="fractal-worker"
CLIENT_NAME="fractal_client"
REPLICAS=2
LOG_FILE="data/metrics_log.csv"

mkdir -p data
echo "Timestamp,Status,Latency_ms,Condition" > $LOG_FILE

echo "--- Starting Performance Analysis Automation ---"

echo "Step 1: Building and Scaling to $REPLICAS workers..."
docker compose up --scale $SERVICE_NAME=$REPLICAS -d
sleep 10 

echo "Step 2: Collecting Baseline Metrics (60s)..."
./$CLIENT_NAME &
CLIENT_PID=$!

sleep 5

sleep 55

echo "Step 3: Simulating Network Disruption (30s)..."

if command -v xdotool &> /dev/null; then
    xdotool key --window $(xdotool search --name "Julia Set Explorer") y
else
    echo "y" > /proc/$CLIENT_PID/fd/0 2>/dev/null
fi
sleep 30

echo "Turning off Network Disruption..."
if command -v xdotool &> /dev/null; then
    xdotool key --window $(xdotool search --name "Julia Set Explorer") y
else
    echo "y" > /proc/$CLIENT_PID/fd/0 2>/dev/null
fi
sleep 5

echo "Step 4: Injecting Failure - Stopping one instance..."
docker compose run --rm grpcurl -plaintext fractal-worker:50051 fractal.FractalService/Shutdown

WORKER_ID=$(docker ps --filter "name=${SERVICE_NAME}-2" -q)
docker stop $WORKER_ID

echo "Worker stopped. Monitoring performance (30s)..."
sleep 30

echo "Step 5: Restarting instance for recovery analysis (30s)..."
docker start $WORKER_ID
sleep 30

echo "Step 6: Test complete. Shutting down..."
kill $CLIENT_PID
docker compose down

echo "--- Analysis Finished. Metrics saved to $LOG_FILE ---"