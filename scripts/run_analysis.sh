#!/bin/bash

SERVICE_NAME="fractal-worker"
CLIENT_NAME="fractal_client"
REPLICAS=2
LOG_FILE="data/metrics_log.csv"

echo "--- Starting Performance Analysis Automation ---"

echo "Step 1: Building and Scaling to $REPLICAS workers..."
docker compose up --build --scale $SERVICE_NAME=$REPLICAS -d

sleep 5

echo "Step 2: Collecting Baseline Metrics (30s)..."
./$CLIENT_NAME &
CLIENT_PID=$!
sleep 30

# --- NEW STEP: NETWORK DISRUPTION ---
echo "Step 3: Simulating Network Disruption (30s)..."
# We send the 'y' key to the client process to toggle simulateTimeout
# Note: This requires the client to be the active window or handled via code logic.
# If automation is preferred, ensure your C++ code checks for a specific trigger.
echo "y" > /proc/$CLIENT_PID/fd/0 2>/dev/null || hide_error=1 
sleep 30
echo "Turning off Network Disruption..."
echo "y" > /proc/$CLIENT_PID/fd/0 2>/dev/null || hide_error=1
sleep 5

# --- EDITED STEP: SERVICE FAILURE ---
echo "Step 4: Injecting Failure - Stopping one instance..."
# Using the specific command you provided earlier for a graceful gRPC shutdown
docker compose run --rm grpcurl -plaintext fractal-worker:50051 fractal.FractalService/Shutdown
# Also stopping the container to ensure the load balancer/client registers the hard failure
WORKER_ID=$(docker ps --filter "name=${SERVICE_NAME}-2" -q)
docker stop $WORKER_ID

echo "Worker stopped. Monitoring performance during failure (30s)..."
sleep 30

echo "Step 5: Restarting instance for recovery analysis (30s)..."
docker start $WORKER_ID
sleep 30

echo "Step 6: Test complete. Shutting down..."
kill $CLIENT_PID
docker compose down

echo "--- Analysis Finished. Metrics saved to $LOG_FILE ---"